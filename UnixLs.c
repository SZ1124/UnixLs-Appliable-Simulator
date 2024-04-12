#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <time.h>

#include "list.h"

int temp = 0;
int flag = 0; // 0 = no flag, 1 = i, 2 = j, 3 = both
int stage = 0;
List* directoryList; // could use a list here.
char* current_directory;
char* parent_directory;
char* home_directory;

void init()
{
    directoryList = List_create();
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {

        current_directory = malloc(strlen(cwd) + 1);
        strcpy(current_directory, cwd);
        //printf("Print cwd current: %s\n", cwd);
        
        home_directory = getenv("HOME");
        printf("Home directory: %s\n", home_directory);

        char* last_slash = strrchr(cwd, '/');

        if(last_slash != NULL)
        {
            *last_slash = '\0';
            printf("Print cwd parent:  %s\n", cwd);

            // if (access(cwd, F_OK) != -1) {
            // printf("Directory exists.\n");

            //     // Check read permission
            //     if (access(cwd, R_OK) != -1) {
            //         printf("Read permission is granted.\n");
            //     } else {
            //         printf("No read permission.\n");
            //     }

            //     // Check write permission
            //     if (access(cwd, W_OK) != -1) {
            //         printf("Write permission is granted.\n");
            //     } else {
            //         printf("No write permission.\n");
            //     }

            //     // Check execute permission
            //     if (access(cwd, X_OK) != -1) {
            //         printf("Execute permission is granted.\n");
            //     } else {
            //         printf("No execute permission.\n");
            //     }
            // } else {
            //     printf("Directory does not exist.\n");
            // }

            // if (access(cwd, F_OK) != -1) {
            //     // Read the target of the symlink
            //     char buf[1024];
            //     ssize_t len = readlink(cwd, buf, sizeof(buf)-1);
            //     if (len != -1) {
            //         buf[len] = '\0';
            //         printf("Symlink '%s' points to: '%s'\n", cwd, buf);

            //         // Check if the target of the symlink exists
            //         if (access(buf, F_OK) == -1) {
            //             printf("Target '%s' does not exist.\n", buf);
            //         }
            //     } else {
            //         perror("readlink");
            //     }
            // } else {
            //     perror("access");
            // }

            parent_directory = malloc(strlen(cwd) + 1);
            strcpy(parent_directory, cwd);

            *last_slash = '/'; // Restore the last slash
        }
    } else {
        perror("getcwd");
    }
}

void readDirectory()
{
    char* directory_path = List_trim(directoryList);
    printf("%s\n", directory_path);
    DIR* d = opendir(directory_path);
    if(d == NULL){
        perror("Error opening directory.\n");
        return;
    }
    struct dirent* dp; 
    struct stat buf; 
    while ((dp = readdir(d)) != NULL) {

        char full_path[1024]; // Adjust the size as needed
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, dp->d_name);
        

        if (stat(full_path, &buf) == -1) {
            perror("Error getting file status.\n");
            continue;
        }
        else {
            if (lstat(full_path, &buf) == -1) {
                perror("Error getting file status.\n");
                continue;
            }
        }

        // if (buf.st_mode & S_IRUSR) {
        //     printf("File is readable.\n");
        // } else {
        //     printf("File is not readable.\n");
        // }
        //printf("%s\n", full_path);

        switch(flag){
            //issue about ls .. -i !
            case 0:
                if(strncmp(dp->d_name, ".", 1) != 0)
                {
                    printf("%s \n", dp->d_name);
                }
                break;

            case 1:
                if(strncmp(dp->d_name, ".", 1) != 0)
                {
                    printf("%ld %s \n", buf.st_ino, dp->d_name);
                }
                break;

            case 2:
                if(strncmp(dp->d_name, ".", 1) != 0)
                {
                    //printf("%s \n", dp->d_name);
                    /*
                    
                        FIX:
                            3. failing to read information about the /home directory in general
                    
                    */
                    //print permission (1st column)
                    if (S_ISLNK(buf.st_mode)) {
                        printf("l");
                    } else if (S_ISDIR(buf.st_mode)) {
                        printf("d");
                    } else {
                        printf("-");
                    }
                    printf((buf.st_mode & S_IRUSR) ? "r" : "-");
                    printf((buf.st_mode & S_IWUSR) ? "w" : "-");
                    printf((buf.st_mode & S_IXUSR) ? "x" : "-");
                    printf((buf.st_mode & S_IRGRP) ? "r" : "-");
                    printf((buf.st_mode & S_IWGRP) ? "w" : "-");
                    printf((buf.st_mode & S_IXGRP) ? "x" : "-");
                    printf((buf.st_mode & S_IROTH) ? "r" : "-");
                    printf((buf.st_mode & S_IWOTH) ? "w" : "-");
                    printf((buf.st_mode & S_IXOTH) ? "x " : "- ");
                    //print # of hard link (2nd column)
                    printf("%lu ", buf.st_nlink);

                    //print the owner of the file (3rd column)
                    struct passwd *pwd1 = getpwuid(buf.st_uid);
                    printf("%s ", pwd1->pw_name);



                    //print the name of the group file belongs to (4th column)
                    struct group *grp1 = getgrgid(buf.st_gid);
                    printf("%s", grp1->gr_name);

                    //print the size of the file in bytes (5th column)
                    printf("%8ld ", buf.st_size); //Size being occupied by 8 characters (changeable) to align numbers to the right
                    
                    //print the date and time of the most recent change (6th column)
                    time_t lastModified1 = buf.st_mtime;
                    struct tm* time1;
                    time1 = localtime(&lastModified1);
                    char buffer1[80];
                    strftime(buffer1, 80, "%b %e %Y %H:%M", time1);
                    printf("%s ", buffer1);

                    //print the name of the file or directory (7th column)
                    printf("%s ", dp->d_name);

                    //print the actual directory of the symbolic link (symbolic link only)
                    if (S_ISLNK(buf.st_mode)) {
                        printf("-> %s \n", full_path);
                    }
                    else{
                        printf("\n");
                    }
                }
                break;

            case 3:
                if(strncmp(dp->d_name, ".", 1) != 0)
                {
                    //print serial number (1st column)
                    printf("%ld ", buf.st_ino); //Size being occupied by 10 characters (changeable) to align numbers to the right

                    //print permission (2nd column)
                    printf("%c", (S_ISDIR(buf.st_mode)) ? 'd' : '-');
                    printf((buf.st_mode & S_IRUSR) ? "r" : "-");
                    printf((buf.st_mode & S_IWUSR) ? "w" : "-");
                    printf((buf.st_mode & S_IXUSR) ? "x" : "-");
                    printf((buf.st_mode & S_IRGRP) ? "r" : "-");
                    printf((buf.st_mode & S_IWGRP) ? "w" : "-");
                    printf((buf.st_mode & S_IXGRP) ? "x" : "-");
                    printf((buf.st_mode & S_IROTH) ? "r" : "-");
                    printf((buf.st_mode & S_IWOTH) ? "w" : "-");
                    printf((buf.st_mode & S_IXOTH) ? "x " : "- ");

                    //print # of hard link (3rd column)
                    printf("%lu ", buf.st_nlink);

                    //print the owner of the file (4th column)
                    struct passwd *pwd2 = getpwuid(buf.st_uid);
                    printf("%s ", pwd2->pw_name);

                    //print the name of the group file belongs to (5th column)
                    struct group *grp2 = getgrgid(buf.st_gid);
                    printf("%s", grp2->gr_name);

                    //print the size of the file in bytes (6th column)
                    printf("%8ld ", buf.st_size); //Size being occupied by 8 characters (changeable) to align numbers to the right
                    
                    //print the date and time of the most recent change (7th column)
                    time_t lastModified2 = buf.st_mtime;
                    struct tm* time2;
                    time2 = localtime(&lastModified2);
                    char buffer2[80];
                    strftime(buffer2, 80, "%b %e %Y %H:%M", time2);
                    printf("%s ", buffer2);

                    //print the name of the file or directory (8th column)
                    printf("%s \n", dp->d_name);
                }
                break;

            default:
                break;
        }
        // You can access other information from 'buf' as needed
    }
    printf("\n");

    closedir(d);
}

void printDirectories()
{
    char* temp;
    List_first(directoryList);
    while(List_curr(directoryList) != NULL){
        temp = List_curr(directoryList);
        printf("%s\n", temp);
        List_next(directoryList);
    }
}

void readInput()
{
    char *input = NULL;
    const char delim[] = " \n";
    size_t len = 0;
    ssize_t read;

    while(1){
        flag = 0;
        stage = 0;
        temp = 0;
        read = getline(&input, &len, stdin);
        if (read == -1) {
            perror("getline");
            exit(EXIT_FAILURE);
        }

        if (strcmp(input, "\n") == 0) {
            break;
        } else {
            char* token = strtok(input, delim);
            if (token == NULL || strcmp(token, "ls") != 0) {
                printf("Invalid input format. Please try again.\n");
                continue;
            } else {
                while ((token = strtok(NULL, delim)) != NULL) {
                    // need to detect, the moment u see 
                    if (token[0] == '-' && stage == 0) {
                        for(int i = 1; i < strlen(token); i++){
                            if(token[i] == 'i'){
                                temp |= 1;
                            } else if (token[i] == 'l'){
                                temp |= 2;
                            } else {
                                stage = 1;
                                temp = 0;
                                List_prepend(directoryList, token);
                                break;
                            }
                        }
                    } else {
                        if(strcmp(token, ".") == 0){
                            List_prepend(directoryList, current_directory);
                            break;
                        }
                        else if(strcmp(token, "..") == 0){
                            List_prepend(directoryList, parent_directory);
                            break;
                        }
                        else if(strcmp(token, "~") == 0){
                            List_prepend(directoryList, home_directory);
                            break;

                        }
                        List_prepend(directoryList, token);
                    } 
                    flag |= temp;
                }
                
                if(List_count(directoryList) == 0){
                    List_prepend(directoryList, current_directory);
                }
            }
            printf("%d\n", flag);
        }
        while(List_count(directoryList) != 0){
            printf("directory: %s\n", (char*)List_curr(directoryList));
            readDirectory();
        } 
    }
}

int main() {
    init();
    readInput();
    return 0;

}
