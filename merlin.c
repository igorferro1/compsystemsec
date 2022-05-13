int fuzz_uid(char *executable)
{
    printf("===== fuzz uid \n");

    // header creation
    struct tar_t *header;
    if ((header = (struct tar_t *)calloc(1, sizeof(struct tar_t))) == NULL)
    {
        ERROR("Unable to malloc header");
        return -1;
    }

    // Test a non ascii character at every position
    for (int pos = 0; pos < 8; pos++)
    {
        char c = (char)128; // first non ascii character chosen

        // Fill in the header
        strcpy(header->name, "uid");
        strcpy(header->mode, "07777");
        header->uid[pos] = c;
        char *content = "Hello World !";
        strcpy(header->size, "015");
        strcpy(header->magic, "ustar"); // TMAGIC = ustar
        strcpy(header->version, "00");
        calculate_checksum(header);

        // Write header and file into archive
        if (tar_write("archive.tar", header, content) == -1)
        {
            ERROR("Unable to write the tar file");
            free(header);
            return -1;
        }

        int rv;
        if ((rv = launches(executable)) == -1)
        {
            ERROR("Error in launches");
            free(header);
            return -1;
        }
        else if (rv == 1)
        // * The program has crashed *
        {
            ERROR("FOUND AN ARCHIVE THAT CRASHED");
            return 1;
        }
    }

    // Test all characters from ASCII table and extended ASCII table in the name: https://ascii-tables.com/
    for (int i = 0; i < 256; i++)
    {
        char c = (char)i;

        // Fill in the header
        strcpy(header->name, "uid");
        strcpy(header->mode, "07777");
        header->uid[0] = c;
        char *content = "Hello World !";
        strcpy(header->size, "015");
        strcpy(header->magic, "ustar"); // TMAGIC = ustar
        strcpy(header->version, "00");
        calculate_checksum(header);

        // Write header and file into archive
        if (tar_write("archive.tar", header, content) == -1)
        {
            ERROR("Unable to write the tar file");
            free(header);
            return -1;
        }
        int rv;
        if ((rv = launches(executable)) == -1)
        {
            ERROR("Error in launches");
            free(header);
            return -1;
        }
        else if (rv == 1)
        // * The program has crashed *
        {
            ERROR("FOUND AN ARCHIVE THAT CRASHED");
            return 1;
        }
    }

    // Test every number at every position
    for (int pos = 0; pos < 8; pos++)
    {
        for (int i = 0; i < 10; i++)
        {
            char c = (char)i;

            // Fill in the header
            strcpy(header->name, "uid");
            strcpy(header->mode, "07777");
            header->uid[pos] = c;
            char *content = "Hello World !";
            strcpy(header->size, "015");

            strcpy(header->magic, "ustar"); // TMAGIC = ustar
            strcpy(header->version, "00");
            calculate_checksum(header);

            // Write header and file into archive
            if (tar_write("archive.tar", header, content) == -1)
            {
                ERROR("Unable to write the tar file");
                free(header);
                return -1;
            }

            int rv;
            if ((rv = launches(executable)) == -1)
            {
                ERROR("Error in launches");
                free(header);
                return -1;
            }
            else if (rv == 1)
            // * The program has crashed *
            {
                printf("--- AN ERRONEOUS ARCHIVE FOUND \n");
                return 1;
            }
        }
    }

    free(header);

    return 0;
}
