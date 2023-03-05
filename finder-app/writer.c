#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[])
{

    openlog("writer", 0, LOG_USER);

    if (argc < 3)
    {

        syslog(LOG_ERR, "Error: Invalid number of arguments\n");
        return 1;
    }
    else
    {

        const char *file = argv[1];
        const char *text = argv[2];

        int fd;
        fd = open(argv[1], O_CREAT | O_RDWR, 0644);

        if (fd == -1)
        {
            syslog(LOG_ERR, "Could not create file\n");
        }
        else
        {
            int written_bytes = write(fd, argv[2], strlen(argv[2]));
            if (written_bytes != strlen(argv[2]))
                syslog(LOG_ERR, "Did not complete write\n");
            else
                syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
        }
        close(fd);
        return 0;
    }
}
