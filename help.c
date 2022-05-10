#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>

/* Source : https : // www.gnu.org/software/tar/manual/html_node/Standard.html */

#define TMAGIC "ustar" /* ustar and a null */
#define TMAGLEN 6
#define TVERSION "00" /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE '0'   /* regular file */
#define AREGTYPE '\0' /* regular file */
#define LNKTYPE '1'   /* link */
#define SYMTYPE '2'   /* reserved */
#define CHRTYPE '3'   /* character special */
#define BLKTYPE '4'   /* block special */
#define DIRTYPE '5'   /* directory */
#define FIFOTYPE '6'  /* FIFO special */
#define CONTTYPE '7'  /* reserved */

#define XHDTYPE 'x' /* Extended header referring to the \
                       next file in the archive */
#define XGLTYPE 'g' /* Global extended header */

/* Header of tar file */
struct tar_header
{                       /* byte offset */
    char name[100];     /*   0 */
    char mode[8];       /* 100 */
    char uid[8];        /* 108 */
    char gid[8];        /* 116 */
    char size[12];      /* 124 */
    char mtime[12];     /* 136 */
    char chksum[8];     /* 148 */
    char typeflag;      /* 156 */
    char linkname[100]; /* 157 */
    char magic[6];      /* 257 */
    char version[2];    /* 263 */
    char uname[32];     /* 265 */
    char gname[32];     /* 297 */
    char devmajor[8];   /* 329 */
    char devminor[8];   /* 337 */
    char prefix[155];   /* 345 */
    char padding[12];   /* 500 */
};
// acima foi dado: não precisa mudar nem mexer nos comentarios, ta no source que eu coloquei la no
struct tar_t
{
    unsigned pos;
    unsigned remaining_data;
    void *openFile;
};

/**
 * Computes the checksum for a tar header and encode it on the header
 * @param entry: The tar header
 * @return the value of the checksum
 */
unsigned int calculate_checksum(struct tar_header *entry)
{
    /* use spaces for the checksum bytes while calculating the checksum */
    memset(entry->chksum, ' ', 8);

    /* sum of entire metadata */
    unsigned int check = 0;
    unsigned char *raw = (unsigned char *)entry;
    for (int i = 0; i < 512; i++)
    {
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
} // DADO: n precisa mexer em nada

/**
 * Launches another axecutable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param the path to the executable
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 *
 * BONUS (for fun, no additional marks) without modifying this code,
 * compile it and use the executable to restart our computer.
 */

/*
    Set each field of the header accordingly to the values and structure that are needed to be set
    name is just a string, so we can use strncpy
    mode, uid, gid are in octal format with 7 chars, so we can use sprintf
    uname and gname are just strings also but we have to limit the size, hence we use sprintf as well
    mtime is the time of data modification, it's also an octal but we'll put 11 chars, we use sprintf
*/

int create_header(struct tar_t *tar_file, struct tar_header header, char *name, unsigned size, unsigned mode, unsigned type, char *version)
{
    memset(&header, 0, sizeof(struct tar_header));
    strncpy(header.name, name, 100);
    sprintf(header.mode, "%07o", mode);
    sprintf(header.uid, "%07o", getuid());
    sprintf(header.gid, "%07o", getgid());
    sprintf(header.uname, "%s", getpwuid(getuid())->pw_name);
    sprintf(header.gname, "%s", getgrgid(getgid())->gr_name);
    sprintf(header.mtime, "%011o", (int)time(NULL));
    memcpy(header.version, version, 2);
    memcpy(header.magic, TMAGIC, 6);
    memset(header.size, '0', 12);
    header.typeflag = type;
    if (type == REGTYPE)
        sprintf(header.size, "%011o", (unsigned)size);

    calculate_checksum(&header);
    tar_file->remaining_data = header.size;
    fwrite(&header, sizeof(header), 1, tar_file->openFile);
    return 0;
} // acho que tá bom de refatorar, mudei algumas funcoes mas esse fim tá igual, acho que n tem oq fazer mais

int fill_with_zeros(struct tar_t *file, int n, bool error)
{
    if (error)
        return 0;
    int i;
    char nil = '\0';
    for (i = 0; i < n; i++)
    {
        if (fwrite(&nil, 1, 1, file->openFile) != 1)
            return -1;
    }
    return 0;
} // tmb não dá pra fugir disso, refatorei oq deu

int create_data(struct tar_t *file, void *data, unsigned size, unsigned nmemb)
{
    fwrite(data, size, nmemb, file->openFile);
    if (file->remaining_data == size)
    {
        file->remaining_data = 0;
        int number_of_zeros = (512 - file->pos % 512) % 512;
        return fill_with_zeros(file, number_of_zeros, 0);
    }
    else
        file->remaining_data = file->remaining_data - size;
    return 0;
} // tmb não dá pra fugir disso, refatorei oq deu

void create_file(char *name, unsigned typeflag, unsigned mode, int errorNumber, char *version)
{

    char *content = "pamonha\n";

    struct tar_t tarFile;
    struct tar_header header;

    (&tarFile)->openFile = fopen(name, "wb");

    create_header(&tarFile, header, "aaa\0.txt", strlen(content), mode, typeflag, version);

    if (errorNumber != 2)
        create_data(&tarFile, content, strlen(content), 1);
    if (!errorNumber)
    {
        fill_with_zeros(&tarFile, sizeof(struct tar_header) * 2, false);
        fclose((&tarFile)->openFile);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No arguments\n");
        return -1;
    } // checar se ele deu isso
    int i, total, rv = 0;
    total = 100;
    for (i = 0; i < total; i++)
    {
        char txt[120];
        char archive[15];
        snprintf(archive, 22, "archive%d.tar", i);

        if (i == 0) // First error: not finishing with two blocks of 512 zeros
            create_file(archive, REGTYPE, 0664, 1, "00");
        else if (i == 1)
            create_file(archive, REGTYPE, 0664, 2, "00");
        else
        {
            char str[2];
            sprintf(str, "%d", i);
            create_file(archive, REGTYPE, 0664, 0, str);
        }

        char cmd[51];
        strncpy(cmd, argv[1], 25);
        cmd[26] = '\0';
        strncat(cmd, " ", 1);
        strncat(cmd, archive, 25);
        char buf[33];
        FILE *fp;

        if ((fp = popen(cmd, "r")) == NULL)
        {
            printf("Error opening pipe!\n");
            return -1;
        }

        if (fgets(buf, 33, fp) == NULL)
        {
            printf("No output\n");
            if (!remove(archive))
                printf("File %s deleted.\n", archive);
            goto finally;
        }
        if (strncmp(buf, "*** The program has crashed ***\n", 33))
        {
            printf("Not the crash message\n");
            goto finally;
        }
        else
        {
            printf("Parabéns zé kkkakkkasdsad vai ser pai de novo\n");
            rv = 1;
            goto finally;
        }
    finally:
        if (pclose(fp) == -1)
        {
            printf("Command not found\n");
            rv = -1;
        }
    }
    return rv;
}
