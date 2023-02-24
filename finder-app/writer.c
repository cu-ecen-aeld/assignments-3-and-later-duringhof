#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    openlog(NULL, LOG_CONS, LOG_USER);

    if (argc < 3)
    {

        syslog(LOG_ERR, "Invalid Number of Arguments (%d)",argc);

        return 1;
    }
    else
    {

        const char *file = argv[1];
        const char *text = argv[2];
        FILE *fd;

        fd = fopen(file, "w");
        syslog(LOG_DEBUG, "Blahblah %d", argc);
        syslog(LOG_DEBUG, "Writing %s to file %s\n", text, file);
        fprintf(fd, "%s\n", text);
        fclose(fd);

        return 0;
    }
}
