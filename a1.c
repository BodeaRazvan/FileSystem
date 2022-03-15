#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_PATH_LENGTH 1000
#define MAX_WORD_LENGTH 1000

int list_directory (char *path, long size_greater, char *name_ends){
    DIR *directory;
    struct dirent *currentDirectory;
    struct stat inode;
    char name[MAX_PATH_LENGTH];
    int endLength=strlen(name_ends);

    directory=opendir(path);
    if (!directory){
        printf("ERROR\n");
        printf("invalid directory path\n");
        return -1;
    }
    while((currentDirectory=readdir(directory))!=0){
       sprintf(name,"%s/%s",path,currentDirectory->d_name);
       if(sizeof(name)<endLength){
           printf("ERROR\n");
           printf("Ending name is longer than the name of the searched element\n");
           return -1;
       }
       lstat(name,&inode);
       if(name[strlen(name)-1]!='.' && name[strlen(name)-2]!='.'){
               if(S_ISDIR(inode.st_mode) && size_greater==-1){
                   if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                       printf("%s\n",name);
               }
               if(S_ISREG(inode.st_mode)){
                   if(size_greater==-1){
                       if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                           printf("%s\n",name);
                   }else
                       if(inode.st_size>size_greater){
                           printf("%s\n",name);
                       }
               }
               if(S_ISLNK(inode.st_mode && size_greater==-1)){
                   if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                       printf("%s\n",name);
               }
           }
    }
    closedir(directory);
    return 0;
}

int list_directory_recursive(char *path, long size_greater, char *name_ends){
    char name[MAX_WORD_LENGTH];
    struct dirent *currentDirectory;
    DIR *directory = opendir(path);
    struct stat inode;
    int endLength=strlen(name_ends);
    if (!directory){
        return -1;
    }

    while ((currentDirectory = readdir(directory)) != NULL){
        if (strcmp(currentDirectory->d_name, ".") != 0 && strcmp(currentDirectory->d_name, "..") != 0){
            sprintf(name, "%s/%s", path, currentDirectory->d_name);
            if(sizeof(name)<endLength){
                printf("ERROR\n");
                printf("Ending name is longer than the name of the searched element\n");
                return -1;
            }
            lstat(name,&inode);
            if(S_ISDIR(inode.st_mode) && size_greater==-1){
                if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                    printf("%s\n",name);
            }
            if(S_ISREG(inode.st_mode)){
                if(size_greater==-1){
                    if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                        printf("%s\n",name);
                }else
                    if(inode.st_size>size_greater){
                        printf("%s\n",name);
                    }
            }
            if(S_ISLNK(inode.st_mode) && size_greater==-1){
                if(strcmp(name+strlen(name)-endLength,name_ends)==0)
                    printf("%s\n",name);
            }
            list_directory_recursive(name,size_greater,name_ends);
        }
    }
    closedir(directory);
    return 0;
}

int parse_section_file(char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    char *magic =malloc(sizeof(char)*4);
    int header_size=0;
    int version=0;
    int noOfSections=0;
    char *sect_name=malloc(sizeof(char)*14);
    int sect_type=0;
    int sect_offset=0;
    int sect_size=0;

    lseek(fd,-4L,SEEK_END);  //last 4 bytes
    read(fd,magic,4); //read MAGIC

    if(strcmp(magic,"UV0t")!=0){  //check if MAGIC is valid
        printf("ERROR\n");
        printf("wrong magic\n");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }

    lseek(fd,-6L,SEEK_END);   //reading header size
    read(fd,&header_size,2);

    lseek(fd,-header_size,SEEK_END);  //because we have the header size, we can go back to the start of the header
    read(fd,&version,4);       //and read normally
    if(version<57 || version>150){
        printf("ERROR\n");
        printf("wrong version");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }
    read(fd,&noOfSections,1);
    if(noOfSections<2 || noOfSections>16){
        printf("ERROR\n");
        printf("wrong sect_nr");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }

    for(int i=0;i<noOfSections;i++){
        read(fd,sect_name,14);
        read(fd,&sect_type,1);
        read(fd,&sect_offset,4);
        read(fd,&sect_size,4);

        if(sect_type!=49 && sect_type!=68 && sect_type!=26 &&sect_type!=82 && sect_type!=75 && sect_type!=23){
            printf("ERROR\n");
            printf("wrong sect_types");
            close(fd);
            free(magic);
            free(sect_name);
            return -1;
        }
    }
    printf("SUCCESS\n");
    printf("version=%d\n",version);
    printf("nr_sections=%d\n",noOfSections);

    //we put the file pointer at the start of the header
    lseek(fd,-header_size,SEEK_END);
    //skip the first 5 bytes that are not from section header (sizeof(Version)+sizeof(No_of_sections))
    lseek(fd,5,SEEK_CUR);

    for(int i=1;i<=noOfSections;i++){
        read(fd,sect_name,14);
        read(fd,&sect_type,1);
        lseek(fd,4,SEEK_CUR);   //we don't display the offset so we jump over it (4 bytes)
        read(fd,&sect_size,4);
        printf("section%d: %s %d %d\n",i,sect_name,sect_type,sect_size);
    }

    close(fd);
    free(sect_name);
    free(magic);
    return 0;
}

int extract(char *path, int section, int line){
    int fd = open(path,O_RDONLY);
    if(fd<0){
        return -1;
    }
    char *magic =malloc(sizeof(char)*4);
    int header_size=0;
    int version=0;
    int noOfSections=0;
    char *sect_name=malloc(sizeof(char)*14);
    int sect_type=0;
    int sect_offset=0;
    int sect_size=0;

    lseek(fd,-4L,SEEK_END);  //last 4 bytes
    read(fd,magic,4); //read MAGIC
    if(strcmp(magic,"UV0t")!=0){  //check if MAGIC is valid
        printf("ERROR\n");
        printf("invalid file\n");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }

    lseek(fd,-6L,SEEK_END);   //reading header size
    read(fd,&header_size,2);

    lseek(fd,-header_size,SEEK_END);  //because we have the header size, we can go back to the start of the header
    read(fd,&version,4);       //and read normally
    if(version<57 || version>150){
        printf("ERROR\n");
        printf("invalid file");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }
    read(fd,&noOfSections,1);
    if(noOfSections<2 || noOfSections>16){
        printf("ERROR\n");
        printf("invalid file");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }

    for(int i=0;i<noOfSections;i++){
        read(fd,sect_name,14);
        read(fd,&sect_type,1);
        read(fd,&sect_offset,4);
        read(fd,&sect_size,4);

        if(sect_type!=49 && sect_type!=68 && sect_type!=26 &&sect_type!=82 && sect_type!=75 && sect_type!=23){
            printf("ERROR\n");
            printf("invalid file");
            close(fd);
            free(magic);
            free(sect_name);
            return -1;
        }
    }

    if(section>noOfSections){
        printf("ERROR\n");
        printf("invalid section\n");
        close(fd);
        free(magic);
        free(sect_name);
        return -1;
    }

    lseek(fd,-header_size,SEEK_END);
    lseek(fd,5,SEEK_CUR);
    for(int i=0;i<section;i++){
        read(fd,sect_name,14);
        read(fd,&sect_type,1);
        read(fd,&sect_offset,4);
        read(fd,&sect_size,4);
    }

    lseek(fd,sect_offset,SEEK_SET); //go to the offset of the section
    char *sectionContent=malloc(sect_size*sizeof(char));
    read(fd,sectionContent,sect_size);  //read the data from that section
    //now we want to find the nr of lines of that section by finding the nr of 0D0A bytes
    int lines=1;
    int section_length=strlen(sectionContent);
    for(int i=0; i<=section_length-1;i++){
        if(sectionContent[i]==0x0D)
            if(sectionContent[i+1]==0x0A)
                lines++;
    }
    //check if the line we want to find exists  line= the line we want to find
    if(line>lines){
        printf("ERROR\n");
        printf("invalid line\n");
        close(fd);
        free(sectionContent);
        free(magic);
        free(sect_name);
        return -1;
    }

    printf("SUCCESS\n");
    int searched_line=lines-line+1;  //we count the lines from end to start
    lines=1;
    int flag=0;
    int end=0;
    int start=0;
    //Initially I tried saving the searched string into a secondary string but with that method I had problems with the time limit so I used flags
    for(int i=0; i<=section_length-1;i++){
        if(sectionContent[i]==0x0D)
            if(sectionContent[i+1]==0x0A)
                lines++;
        if(lines==searched_line){
            if(flag==0){
                start=i;
                flag=1;
            }
            if(sectionContent[i+1]==0x0D && sectionContent[i+2]==0x0A){
                end=i;
                break;
            }
            if(sectionContent[i+1]=='\0'){
                end=i;
                break;
            }
        }
    }
    //printing the string reversed between end and start
    for(int i=end;i>=start;i--){
        printf("%c",sectionContent[i]);
    }

    free(sectionContent);
    close(fd);
    free(magic);
    free(sect_name);
    return 0;
}

int findall(char *path){
    char name[MAX_WORD_LENGTH];
    struct dirent *currentDirectory;
    DIR *directory = opendir(path);
    struct stat inode;
    if (!directory){
        return -1;
    }

    while ((currentDirectory = readdir(directory)) != NULL){
        if (strcmp(currentDirectory->d_name, ".") != 0 && strcmp(currentDirectory->d_name, "..") != 0){
            sprintf(name, "%s/%s", path, currentDirectory->d_name);
            lstat(name,&inode);
            if(S_ISDIR(inode.st_mode)){
                   findall(name);
            }
            if(S_ISREG(inode.st_mode)){
                int sizeFlag=1;
                int fd = open(name,O_RDONLY);
                if(fd<0){
                    return -1;
                }
                char *magic=malloc(4*sizeof(char));
                int header_size=0;
                int version=0;
                int noOfSections=0;
                char *sect_name=malloc(14*sizeof(char));
                int sect_type=0;
                int sect_offset=0;
                int sect_size=0;

                lseek(fd,-4,SEEK_END);  //last 4 bytes
                read(fd,magic,4); //read MAGIC
                if(strncmp(magic,"UV0t",4)!=0){  //check if MAGIC is valid
                     sizeFlag=0;
                }
                free(magic);

                lseek(fd,-6L,SEEK_END);   //reading header size
                read(fd,&header_size,2);

                lseek(fd,-header_size,SEEK_END);  //because we have the header size, we can go back to the start of the header
                read(fd,&version,4);       //and read normally
                if(version<57 || version>150){
                    sizeFlag=0;
                }
                read(fd,&noOfSections,1);
                if(noOfSections<2 || noOfSections>16){
                   sizeFlag=0;
                }

                for(int i=0;i<noOfSections;i++){
                    read(fd,sect_name,14);
                    read(fd,&sect_type,1);
                    read(fd,&sect_offset,4);
                    read(fd,&sect_size,4);

                    if(sect_type!=49 && sect_type!=68 && sect_type!=26 &&sect_type!=82 && sect_type!=75 && sect_type!=23){
                       sizeFlag=0;
                    }
                    if(sect_size>1258){
                        sizeFlag=0;
                    }
                }
                free(sect_name);
                if(sizeFlag)
                    printf("%s\n",name);
            }
        }
    }
    closedir(directory);
    return 0;
}



int main(int argc, char **argv){
    long size_greater=-1;
    int recursive=0;
    char name_ends[MAX_WORD_LENGTH]={ '\0' };
    char path[MAX_PATH_LENGTH];
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("88815\n");
            return -1;
        }
        if(strcmp(argv[1], "list") == 0){
            if(argc < 3){
                printf("ERROR\n");
                printf("Too few arguments for command list\n");
                return -1;
            }
            if(argc == 3){
                // ./a1 list path=.....
                if(strncmp(argv[2],"path=",5)==0){
                    memmove(path,argv[2]+5,MAX_PATH_LENGTH);   // we get rid of path= part
                }else{
                    printf("ERROR\n");
                    printf("Path was not specified\n");
                    return -1;
                }
            }
            if(argc == 4){
                if(strncmp(argv[2], "path=",5)==0){
                    memmove(path,argv[2]+5,MAX_PATH_LENGTH);
                }else
                if(strncmp(argv[3],"path=",5)==0){
                    memmove(path,argv[3]+5,MAX_PATH_LENGTH);
                }else{
                    printf("ERROR\n");
                    printf("Path was not specified\n");
                    return -1;
                }
               if(strcmp(argv[2],"recursive")==0 || strcmp(argv[3],"recursive")==0){
                   recursive=1;
               }
               if(strncmp(argv[2],"size_greater=",13)==0){
                   size_greater=atoi(argv[2]+13);
               }
               if(strncmp(argv[3],"size_greater=",13)==0){
                   size_greater=atoi(argv[3]+13);
               }
               if(strncmp(argv[2],"name_ends_with=",15)==0){
                   memmove(name_ends,argv[2]+15,MAX_WORD_LENGTH);
               }
               if(strncmp(argv[3],"name_ends_with=",15)==0){
                   memmove(name_ends,argv[3]+15,MAX_WORD_LENGTH);
               }
            }
            if(argc==5){
                //   ./a1 list   recursive, path, options
                if(strncmp(argv[2], "path=",5)==0){
                    memmove(path,argv[2]+5,MAX_PATH_LENGTH);
                }else
                if(strncmp(argv[3],"path=",5)==0){
                    memmove(path,argv[3]+5,MAX_PATH_LENGTH);
                }else
                    if(strncmp(argv[4],"path=",5)==0){
                        memmove(path,argv[4]+5,MAX_PATH_LENGTH);
                }else{
                        printf("ERROR\n");
                        printf("No path specified\n");
                        return -1;
                    }
               if(strcmp(argv[2],"recursive")==0 || strcmp(argv[3],"recursive")==0 || strcmp(argv[4],"recursive")==0 ){
                    recursive=1;
               }
                if(strncmp(argv[2],"size_greater=",13)==0){
                    size_greater=atol(argv[2]+13);
                }
                if(strncmp(argv[3],"size_greater=",13)==0){
                    size_greater=atol(argv[3]+13);
                }
                if(strncmp(argv[4],"size_greater=",13)==0){
                    size_greater=atol(argv[4]+13);
                }
                if(strncmp(argv[2],"name_ends_with=",15)==0){
                    memmove(name_ends,argv[2]+15,MAX_WORD_LENGTH);
                }
                if(strncmp(argv[3],"name_ends_with=",15)==0){
                    memmove(name_ends,argv[3]+15,MAX_WORD_LENGTH);
                }
                if(strncmp(argv[4],"name_ends_with=",15)==0){
                    memmove(name_ends,argv[4]+15,MAX_WORD_LENGTH);
                }
            }
            if(argc > 5){  // I did not consider the possibility of having 2 extra options at the same time (not mandatory)
                printf("ERROR\n");
                printf("Too many arguments for command list\n");
                return -1;
            }

            struct stat fileMetadata;
            if(stat(path,&fileMetadata)<0){
                printf("ERROR\n");
                printf("invalid directory path\n");
                return -1;
            }
            if(S_ISDIR(fileMetadata.st_mode)==0){
                printf("ERROR\n");
                printf("invalid directory path\n");
                return -1;
            }
            printf("SUCCESS\n");


            if(recursive==0){
                list_directory(path,size_greater,name_ends);
            }else{
                list_directory_recursive(path,size_greater,name_ends);
            }

            /*   Just testing code
            printf("Path: %s\n",path);
            printf("Recursive: %d\n",recursive);
            printf("Size greater: %d\n",size_greater);
            printf("Name ends with: %s\n",name_ends);
            if(name_ends[0]=='\0')
                printf("Empty\n");
            */
        }
        //Start of task 2.4
        if(strcmp(argv[1],"parse")==0){
            if(argc == 2 || argc>3){
                printf("ERROR\n");
                printf("Nr of arguments is incorrect\n");
                return -1;
            }
            if(strncmp(argv[2],"path=",5)==0) {
                memmove(path, argv[2] + 5, MAX_PATH_LENGTH);
            }else{
                printf("ERROR\n");
                printf("Path was not given, try path= \n");
                return -1;
            }
            parse_section_file(path);
        }
        if(strcmp(argv[2],"parse")==0){
            if(argc == 2 || argc>3){
                printf("ERROR\n");
                printf("Nr of arguments is incorrect\n");
                return -1;
            }
            if(strncmp(argv[1],"path=",5)==0) {
                memmove(path, argv[1] + 5, MAX_PATH_LENGTH);
            }else{
                printf("ERROR\n");
                printf("Path was not given, try path= \n");
                return -1;
            }
            parse_section_file(path);
        }
        if(strcmp(argv[1],"extract")==0){
            if(argc!=5){
                printf("ERROR\n");
                printf("Incorrect nr of arguments for extract.Expected:%d, given:%d\n",5,argc);
                exit(-1);
            }
            if(strncmp(argv[2],"path=",5)!=0){
                printf("ERROR\n");
                printf("Path not given\n");
                return -1;
            }
            if(strncmp(argv[3],"section=",8)!=0){
                printf("ERROR\n");
                printf("Section not given\n");
                return -1;
            }
            if(strncmp(argv[4],"line=",5)!=0){
                printf("ERROR\n");
                printf("Line not given\n");
                return -1;
            }
            memmove(path,argv[2]+5,MAX_WORD_LENGTH);
            int section=atoi(argv[3]+8);
            int line =atoi(argv[4]+5);

            extract(path,section,line);
        }
        if(strcmp(argv[1],"findall")==0){
            if(strncmp(argv[2],"path=",5)==0){
                memmove(path,argv[2]+5,MAX_PATH_LENGTH);
                DIR *directory = opendir(path);
                if (!directory ){
                    printf("ERROR\n");
                    printf("invalid directory path\n");
                    return -1;
                }
                printf("SUCCESS\n");
                findall(path);
                closedir(directory);
            }else{
                printf("ERROR\n");
                printf("Path was not specified\n");
                return -1;
            }
        }
        if(strcmp(argv[1],"variant")!=0 && strcmp(argv[1],"list")!=0 && (strcmp(argv[1],"parse")!=0 && strcmp(argv[2],"parse")!=0) && strcmp(argv[1],"extract")!=0 && strcmp(argv[1],"findall")!=0){
            printf("ERROR\n");
            printf("Command is not existent\n");
            return -1;
        }
    }else{
        printf("ERROR\n");
        printf("Too few arguments\n");
        return -1;
    }
    return 0;
}
