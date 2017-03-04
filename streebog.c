/*
 * Streebog.c
 * Gost3411-2012 checksum utility
 * Author: Michele Pes
 * License: GPL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <gost3411-2012-core.h>


#define BUFFER_SIZE (64 * 1024)

#define EXIT_OK             0
#define EXIT_SYNTAX_ERROR   1
#define EXIT_MEM_ERROR      2


const char* basename(const char* name)
{
    const char* p = name;
    if (p) {
        int i;
        char c;
        for (i = 0; (c = name[i]) && c != '\0'; i++)
            if (c == '\\' || c == '/' || c == ':')
                p = name + i + 1;

    }
    return p;
}

static void help(void)
{
    puts("Enter streebog [-256] <file[s]>\n-256 Set 256-bit hash, else 512");
    puts("Enter - for stdin");
}

static void get_hex_chars(unsigned char c,
                          unsigned char* p_c1, unsigned char* p_c2)
{
    static const unsigned char table[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f'
    };
    if (p_c1)
        *p_c1 = table[c >> 4];
    if (p_c2)
        *p_c2 = table[c & 0xF];
}

void get_hash_string(const void* src, void* dest, unsigned int bits)
{
    const unsigned char* hash = (const unsigned char*)src;
    const unsigned int bytes = bits / 8;
    unsigned char* p = dest;
    unsigned char c1, c2;
    unsigned int i;

    for (i = 0; i < bytes; i++) {
        get_hex_chars(hash[i], &c1, &c2);
        *p++ = c1;
        *p++ = c2;
    }

    *p = '\0';
}

static void**
amalloc(size_t align, const size_t* sizes, size_t count, size_t* p_allocated)
{
    void** ret = NULL;
    if (sizes != NULL && count > 0 && align > 0) {
        size_t i = 0, len = 0;
        do len += sizes[i]; while (++i < count);
        len = len + (count * (align - 1));
        len = len + (count * sizeof(void*));
        if (p_allocated)
            *p_allocated = len;
        ret = malloc(len);
        if (ret) {
            size_t rem, ptr = (size_t)(ret + count);
            i = 0;
            do {
                if (align > 1) {
                    rem = ptr % align;
                    if (rem)
                        ptr = ptr + align - rem;
                }
                ret[i] = (void*)ptr;
                ptr += sizes[i];
            } while (++i < count);
        }
    }
    return ret;
}

int main(int argc, char* argv[])
{
    size_t sizes[] = { BUFFER_SIZE, sizeof(GOST34112012Context), 0, 0 };
    void *io_buf, *hash_str, *hash;
    unsigned int hash_size = 512;
    GOST34112012Context* p_ctx;
    size_t allocated;
    void** mem;
    int i = 1;

    if (argc < 2) {
        help();
        return EXIT_SYNTAX_ERROR;
    }

    if (strcmp(argv[1], "-256") == 0) {
        if (argc < 3) {
            help();
            return EXIT_SYNTAX_ERROR;
        }
        hash_size = 256;
        i++;
    }

    sizes[2] = ((hash_size / 8) * 2) + 1;
    sizes[3] = hash_size / 8;

    mem = amalloc(128, sizes, 4, &allocated);

    if (mem == NULL) {
        puts("No memory: Unable to allocate memory");
        return EXIT_MEM_ERROR;
    }
    io_buf = *mem;
    p_ctx = mem[1];
    hash_str = mem[2];
    hash = mem[3];

    setvbuf(stdout, NULL, _IONBF, 0);

    for (; i < argc; i++) {
        const char* name = argv[i];
        int error, is_file;
        FILE* handle;
        size_t red;

        is_file = strcmp(name, "-");
        if (!is_file) {
            #ifdef _WIN32
            setmode(fileno(stdin), O_BINARY);
            #endif
            handle = stdin;
            name = "stdin";
        }
        else
            handle = fopen(name, "rb");

        printf("(%s)= ", basename(name));

        if (handle == NULL)
            puts("Unable to open file");
        else {
            GOST34112012Init(p_ctx, hash_size);

            do {
                red = fread(io_buf, 1, BUFFER_SIZE, handle);
                error = ferror(handle);
                if (!error)
                    GOST34112012Update(p_ctx, io_buf, red);
            } while (red == BUFFER_SIZE && !error);

            if ((is_file && (fclose(handle) != 0)) || error != 0)
                puts("Unable to read file");
            else {
                GOST34112012Final(p_ctx, hash);
                get_hash_string(hash, hash_str, hash_size);
                puts(hash_str);
            }
        }
    }

    GOST34112012Cleanup(p_ctx); /* Redundant */
    memset((void*)mem, 0, allocated);
    free((void*)mem);

    return EXIT_OK;
}
