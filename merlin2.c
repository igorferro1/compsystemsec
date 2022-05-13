int fuzz_size(char *executable)
{
    printf("===== fuzz size \n");

    // header creation
    struct tar_t *header;
    if ((header = (struct tar_t *)calloc(1, sizeof(struct tar_t))) == NULL)
    {
        ERROR("Unable to malloc header");
        return -1;
    }

    // Test all octal value at all position
    for (int pos = 0; pos < 12; pos++)
    {
        for (int i = 0; i < 8; i++)
        {
            char c = i + 48; // convert int to char

            // Fill in the header
            strcpy(header->name, "size");
            strcpy(header->mode, "07777");
            char *content = "Hello World !";
            header->size[pos] = c;

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

    // Test every ascii and non ascii character at position 0
    for (int i = 0; i < 256; i++)
    {
        char c = (char)i;
        // Fill in the header
        strcpy(header->name, "size");
        strcpy(header->mode, "07777");
        char *content = "Hello World !";
        header->size[0] = c;

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

    // Test a non ascii character at every position
    for (int pos = 0; pos < 12; pos++)
    {
        char c = (char)128; // first non ascii character chosen

        // Fill in the header
        strcpy(header->name, "size");
        strcpy(header->mode, "07777");
        char *content = "Hello World !";
        header->size[pos] = c;

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

    free(header);

    return 0;
}