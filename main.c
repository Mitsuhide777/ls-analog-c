#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#define TEXT_STYLE_DIR "\x1b[34;1m"
#define TEXT_STYLE_LINK "\x1b[36;1m"
#define TEXT_STYLE_RESET "\x1b[m"

char* concat(const char *s1, const char *s2)
{
	// +2 for zero-terminator and '/'
	char *result = malloc(strlen(s1) + strlen(s2) + 2);
	strcpy(result, s1);
	result[strlen(s1)] = '/';
	result[strlen(s1) + 1] = '\0';
	strcat(result, s2);
	return result;
}

void printInfo(char path[], struct dirent* ent)
{
	// for invisible files
	if (ent->d_name[0] == '.')
		return;

	struct stat fileStat; // for state of a file
	struct stat outputStream; // to check whether output stream is terminal or not

	struct passwd *pwd; // to get user name
	struct group *grp; // to get group name

	lstat(concat(path, ent->d_name), &fileStat);
	int isOk = fstat(fileno(stdout), &outputStream);

	if (isOk == -1)
	{
		printf("Some kind of error occured with file \"%s\".", ent->d_name);
		return;
	}

    // File types:
	if (S_ISDIR(fileStat.st_mode))
		printf("d");
	else if (S_ISLNK(fileStat.st_mode))
		printf("l");
	else if (S_ISFIFO(fileStat.st_mode))
		printf("p");
	else if (S_ISSOCK(fileStat.st_mode))
		printf("s");
	else if (S_ISCHR(fileStat.st_mode))
		printf("c");
	else if (S_ISBLK(fileStat.st_mode))
		printf("b");
	else
		printf("-");

    // Permissions:
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");

    // Number of hard links:
    printf("  %2lu", fileStat.st_nlink);

    // User name:
    if ((pwd = getpwuid(fileStat.st_uid)) != NULL)
		printf("  %s", pwd->pw_name);
	else
		printf("  %d", fileStat.st_uid);

	//Group name:
	if ((grp = getgrgid(fileStat.st_gid)) != NULL)
		printf("  %s", grp->gr_name);
	else
		printf("  %d", fileStat.st_gid);

    // File size:
    printf("  %6ld", fileStat.st_size);

    // All to print shorter date:
    struct tm* timeInfo;
    timeInfo = localtime(&fileStat.st_mtime);

    if (!(timeInfo->tm_mon >= 1 && timeInfo->tm_mon <= 12))
    	timeInfo = localtime(&fileStat.st_atime);
    char* month;
    switch(timeInfo->tm_mon)
    {
    	case 1 : month = "Jan"; break;
    	case 2 : month = "Feb"; break;
    	case 3 : month = "Mar"; break;
    	case 4 : month = "Apr"; break;
    	case 5 : month = "May"; break;
    	case 6 : month = "Jun"; break;
    	case 7 : month = "Jul"; break;
    	case 8 : month = "Aug"; break;
    	case 9 : month = "Sep"; break;
    	case 10 : month = "Oct"; break;
    	case 11 : month = "Nov"; break;
    	case 12 : month = "Dec"; break;
    	default : month = "Month"; break;
    }
    // printf("  %s", ctime(&fileStat.st_mtime));
    printf("  %02d %s %02d:%02d  ", timeInfo->tm_mday, month,
    	timeInfo->tm_hour, timeInfo->tm_min);

    // To print path to the real file, if working with a link
    if (S_ISLNK(fileStat.st_mode))
    {
    	char link_path[PATH_MAX];
    	ssize_t length = readlink(concat(path, ent->d_name), 
    		link_path, sizeof(link_path) - 1);
    	if (length != -1)
    		link_path[length] = '\0';

        // Also if output stream is terminal, paint it
    	if (S_ISCHR(outputStream.st_mode))
    	{
    		printf(TEXT_STYLE_LINK "%s" TEXT_STYLE_RESET, ent->d_name);
    		printf(" -> %s\n", link_path);
    	}
    	else
    		printf("  %s -> %s\n", ent->d_name, link_path);
    }
    else
    {
    	// If output stream is terminal, paint it
    	if (S_ISDIR(fileStat.st_mode) && S_ISCHR(outputStream.st_mode))
    		printf(TEXT_STYLE_DIR "%s\n" TEXT_STYLE_RESET, ent->d_name);
    	else
    		printf("%s\n", ent->d_name);
    }
}

void read2(char path[])
{
	DIR* dir;
	struct dirent* ent;

	dir = opendir(path);
	if (dir == NULL)
	{
		printf("Wrong path!\n");
		exit(1);
	}

	while ((ent = readdir(dir)) != NULL)
	{
		printInfo(path, ent);
	}
	closedir(dir);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
		read2(".");
	else
		for (int i = 1; i < argc; ++i)
	    {
		    printf("%s:\n", argv[i]);
		    read2(argv[i]);
	    }
	return 0;
}