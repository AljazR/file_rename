#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h> 
#include <errno.h> 
#include <string.h>

// "rename" switchs slovenian letters in names of files with english ones and spaces with '_'

// USEAGE:
// "./rename <path>" 
// "./rename r <path>" --> recursively changes all file names in given path
// "./rename" --> uses cwd
// "./rename r" --> recursively changes all file names in cwd

int already_exists (char* path, char* new_name) { // checks if any file in directory has same name as new_name
    DIR *directory;
    if ((directory = opendir(path)) == NULL) { // open directory
        perror(__FILE__);
        exit(errno);
    }

    errno = 0;
    struct dirent *file;
    while ((file = readdir(directory)) != NULL) { // loop over files in directory and compare their names with new_name
        if (strcmp(file->d_name, new_name) == 0) {
            return 1;
        }       
    }
    if (errno != 0) {
        perror(__FILE__);
        exit(errno);
    }

    if (closedir(directory) < 0) { // close directory
         perror(__FILE__);
         exit(errno);
    }

    return 0;
}
 
int rename (char* path, int rec) { // main function for correcting names of files
    DIR *directory;
    if ((directory = opendir(path)) == NULL) { // open directory
        perror(__FILE__);
        exit(errno);
    }

    errno = 0;
    struct dirent *file;
    while ((file = readdir(directory)) != NULL) { // loop over files in directory    
        if (strcmp("..", file->d_name) == 0 || strcmp(".", file->d_name) == 0) continue; // don't check parent and current directory because recursion goes brrrrrrrrrrrr 
        
        int len = strlen(file->d_name);
        char new_name[len];
        int new_len = 0; // updates the length of new_name

        for (int i = 0; i < len; i++, new_len++) { // loop over file name and switch non-english characters with english ones and spaces with '_'
            // because UTF-8 represents characters, which we want to switch, with two bytes, we have to check both and replace them with just one
            if ((file->d_name[i] == -60 && file->d_name[i+1] == -115) || (file->d_name[i] == -60 && file->d_name[i+1] == -121)) {
                new_name[new_len] = 'c';
                i++;
            } else if ((file->d_name[i] == -60 && file->d_name[i+1] == -116) || (file->d_name[i] == -60 && file->d_name[i+1] == -122)) {
                new_name[new_len] = 'C';
                i++;
            } else if (file->d_name[i] == -60 && file->d_name[i+1] == -111) {
                new_name[new_len] = 'd';
                i++;
            } else if (file->d_name[i] == -60 && file->d_name[i+1] == -112) {
                new_name[new_len] = 'D';
                i++;
            } else if (file->d_name[i] == -59 && file->d_name[i+1] == -95) {
                new_name[new_len] = 's';
                i++;
            } else if (file->d_name[i] == -59 && file->d_name[i+1] == -96) {
                new_name[new_len] = 'S';
                i++;
            } else if (file->d_name[i] == -59 && file->d_name[i+1] == -66) {
                new_name[new_len] = 'z';
                i++;
            } else if (file->d_name[i] == -59 && file->d_name[i+1] == -67) {
                new_name[new_len] = 'Z';
                i++;
            } else if (file->d_name[i] == ' ') {
                new_name[new_len] = '_';
            } else {
                new_name[new_len] = file->d_name[i];
            }
        }
        new_name[new_len] = '\0'; // add end of string

        if (strcmp(file->d_name, new_name) != 0) { // the name was changed
            
            int i = 2; // counter for duplicated files
            char new_name_temp[len + 10]; // temporary new 
            strcpy(new_name_temp, new_name);
            while (already_exists(path, new_name_temp) != 0) { // check if file with same name already exists (rename overwrites old file with same name...)
                sprintf(new_name_temp,"%s_(%d)", new_name, i); // add "_(<i>)" at the end of the name
                i++;
            }
            strcpy(new_name, new_name_temp);

            char old_path[PATH_MAX];
            char new_path[PATH_MAX];
            sprintf(old_path,"%s/%s", path, file->d_name); // prepare old path for rename
            sprintf(new_path,"%s/%s", path, new_name); // prepare new path for rename
            if (rename(old_path, new_path) < 0) { // rename file
                perror(__FILE__);
                exit(errno);
            }

            if (file->d_type == DT_DIR && rec == 1) { // check if file is a directory and call recursively
                rename(new_path, 1);
            }

        } else if (file->d_type == DT_DIR && rec == 1) { // check if file is a directory and call recursively
            char old_path[PATH_MAX];
            sprintf(old_path,"%s/%s", path, file->d_name);
            rename(old_path, 1);
        }
    }
    if (errno != 0) {
        perror(__FILE__);
        exit(errno);
    }

    if (closedir(directory) < 0) { // close directory
         perror(__FILE__);
         exit(errno);
    }

    return 0;
}

int main (int argc, char* argv[]) {

    if (argc > 3) { // check for too many arguments
        errno = 7;
        perror(argv[0]);
        exit(errno);
    } else if (argc == 2) {
        if (strcmp(argv[1], "r") == 0) { // recursion is on and use cwd
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                rename(cwd, 1);
            } else {
                perror(argv[0]);
                exit(errno);
            }
        } else {
            rename(argv[1], 0); // the path is given
        }

    } else if (argc == 1) { // use cwd
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            rename(cwd, 0);
        } else {
            perror(argv[0]);
            exit(errno);
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "r") == 0) {
            rename(argv[2], 1); // recursion is on and the path is given
        }
    }

    return 0;
}