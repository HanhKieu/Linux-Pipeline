#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <sys/wait.h>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;


void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    //char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

void findFile(string givenPath, string stringToFind){
    DIR *myDir;
    struct dirent *currentFile;
    struct stat statBuf;
    string tempString;
    string path;
    myDir = opendir(givenPath.c_str());
    currentFile = readdir(myDir);

    while(currentFile != NULL){
        //write(STDOUT_FILENO, "\n", 1);
        string tempString(currentFile->d_name);
        //cout << tempString << endl;
        string filePath(givenPath + "/" + tempString);
        stat(filePath.c_str(), &statBuf);

        if(statBuf.st_mode & S_IFDIR && (tempString != ".") && (tempString != "..") ){
            findFile(filePath, stringToFind);
        }
        else if(currentFile->d_name == stringToFind){
            write(STDOUT_FILENO, "\n", 1);
            write(STDOUT_FILENO, filePath.c_str(), filePath.size());
        }

        currentFile = readdir(myDir);
    }

}
void printWorkingDirectory(){
    char buffer[1024];
    char *currentDir = getcwd(buffer, sizeof(buffer));
    string tempString(currentDir);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, tempString.c_str(), tempString.size());
}

void lsStringGenerator(string path){
    struct stat statBuf;
    //scout << stat(currentFile) << endl;
    stat(path.c_str(), &statBuf);
    if(statBuf.st_mode & S_IFDIR)
        write(STDOUT_FILENO, "d", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IRUSR)
        write(STDOUT_FILENO, "r", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IWUSR)
        write(STDOUT_FILENO, "w", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IXUSR)
        write(STDOUT_FILENO, "x", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IRGRP)
        write(STDOUT_FILENO, "r", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IWGRP)
        write(STDOUT_FILENO, "w", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IXGRP)
        write(STDOUT_FILENO, "x", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IROTH)
        write(STDOUT_FILENO, "r", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IWOTH)
        write(STDOUT_FILENO, "w", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    if(statBuf.st_mode & S_IXOTH)
        write(STDOUT_FILENO, "x", 1);
    else
        write(STDOUT_FILENO, "-", 1);
    write(STDOUT_FILENO, " ", 1);
}

void myLs(vector<string> currentLineVec){
    DIR *myDir;
    struct dirent *currentFile;
    string tempString;
    string argument;
    if(currentLineVec.size() > 1){
        myDir = opendir(currentLineVec.at(1).c_str());
        argument = currentLineVec.at(1);
    }
    else{
        myDir = opendir(".");
        argument = ".";
    }

    currentFile = readdir(myDir);

    while(currentFile != NULL){
        write(STDOUT_FILENO, "\n", 1);
        string tempString(currentFile->d_name);
        lsStringGenerator( argument + "/" + tempString);
        write(STDOUT_FILENO, tempString.c_str(), tempString.size());
        currentFile = readdir(myDir);
    }
}

void myFork(vector<string> currentLineVec){ 
    int status;
    string path;
    pid_t my_pid = fork();
    string command = *(currentLineVec.begin());

    if(my_pid == 0){
        if(command == "ls"){
            myLs(currentLineVec);
        }

        else if(command == "pwd"){
            printWorkingDirectory();
        }
        else if(command == "ff"){
            if(currentLineVec.size() == 1){
                string tempString("ff command requires a filename!");
                write(STDOUT_FILENO, "\n", 1);
                write(STDOUT_FILENO, tempString.c_str(), tempString.size());
            }
            else{
                if(currentLineVec.size() >= 3){
                    path = currentLineVec.at(2);
                }
                else{
                    path = ".";
                }
                findFile(path, currentLineVec.at(1));
            }
        else{
            int count = 0;
            vector<string>::iterator itr;

            itr = currentLineVec.begin();
            while(itr != currentLineVec.end()){
                count++;
                itr++;
            } //COUNTS NUMBER OF COMMANDS TO INIT THE CHAR** WITH 

            char* currentLine[count+1];
            for(int i = 0; i < count; i++){
                currentLine[i] = const_cast<char*>(currentLineVec.at(i).c_str());
            }

            currentLine[count] = NULL;

            execvp(currentLine[0], currentLine);
        }
        exit(0);
    }
    else{
        //wait for child to execute waitpid()
        waitpid(my_pid, &status, 0);
        //cout << "okay this ended" << endl;
    }
}

void myCd(vector<string> currentLineVec){
    string errorMessage = "Error changing directory.";
    if(currentLineVec.size() > 1){
        //cd to first argument
        if((chdir(currentLineVec.at(1).c_str()))){
            //unsuccesful cd
            write(STDOUT_FILENO, "\n", 1);
            write(STDOUT_FILENO, errorMessage.c_str(), errorMessage.size());
        }
            
    }
    else{
        //cd to home
        chdir(getenv("HOME"));
    }
        
}

void parseCommand(string currentLineUnparsed){
    stringstream ss(currentLineUnparsed); //parses based on space
    string command;
    string token;
    vector<string> currentLineVec; //additional arguments
    vector<string>::iterator itr;


    while(ss >> token){
        currentLineVec.push_back(token);

    }

    //IF EMPTY LINE ENTERED
    if(currentLineVec.size() < 1)
        return;
    //ELSE SEG FAULT

    command = *(currentLineVec.begin());

    if(command == "cd"){
        myCd(currentLineVec);
//        cout << "DON'T NEED TO FORK" << endl;
    }
    else if(command == "exit"){
        write(STDOUT_FILENO, "\n", 1);
        exit(0);
    }
    else{
        myFork(currentLineVec);
    }
    //itr = arguments.begin();
    // while(itr != arguments.end()){
    //     // if(!((itr->c_str).indexOf('|')))
    //     //     cout << "there's a pipe in here" << endl;
    //     itr++;
    // }

    //call function to run command here

}

void clearSTDOUT(int &sizeOfString, char *currentLine ){
    string deleteString("\b \b");
    
    for(int i = 0; i < sizeOfString ; i++){
        write(STDOUT_FILENO, deleteString.c_str(), 3);
        currentLine[i] = 0;
    }
    sizeOfString = 0;
}

void printCurrentDir(){ //http://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
    char buffer[1024];
    char *currentDir = getcwd(buffer, sizeof(buffer));
    
    if(strlen(currentDir) < 16){
        for(int i = 0; i < strlen(currentDir); i++){
            write(STDOUT_FILENO, &currentDir[i], 1);
        }
    }

    else{//total path is more than 16 chars
//stackoverflow.com/questions/32822988/get-the-last-token-of-a-string-in-c
        const char delimiter[2] = "/";
        char *token, *last;
        last = token = strtok(currentDir, "/");

    
        for(; (token = strtok(NULL, "/")) != NULL; last = token);

        write(STDOUT_FILENO, "/.../", 5);

        for(int i = 0; i < strlen(last); i++)
            write(STDOUT_FILENO, &last[i], 1);
    }

    write(STDOUT_FILENO, "% ", 2);
}

int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    char currentLine[10000];
    int currentLineSize = 0;
    vector<string> myVector;
    vector<string>::iterator it2; //vector iterator
    int numPrevCommands = 0;
    string deleteString("\b \b");
    string soundString("\a");
    int historyCounter = 0;
    bool upArrowOnce = false;
    bool downArrowOnce = false;
    bool firstTimeVisitingBegin = true;
    
//    string

    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    printCurrentDir();
    
    while(1){
        //myLS();
        read(STDIN_FILENO, &RXChar, 1);

        //IF IT IS A PRINTABLE CHARACTER
        if(isprint(RXChar)){ // || RXChar == 0x0A){
            write(STDOUT_FILENO, &RXChar, 1);
            currentLine[currentLineSize++] = RXChar;
        }//WRITE IT OUT TO THE SCREEN, THEN ADD THAT CHARACTER INTO A LIST

        //IF CTRL+D IS TAKEN AS AN INPUT, THEN EXIT
        if(0x04 == RXChar){
            break;
        }
        //IF YOU PRESS BACKSPACE
        else if(0x7F == RXChar){
            
            if(currentLineSize >= 1){
                write(STDOUT_FILENO, deleteString.c_str(), 3);
                currentLineSize--;   
            }

            currentLine[currentLineSize] = 0;
        }
        //OTHERWISE
        else{

            //IF ' '
            if(0x1b == RXChar){

                read(STDIN_FILENO, &RXChar, 1);

                //IF '['
                if(RXChar == 0x5b){
                    read(STDIN_FILENO, &RXChar, 1);

                    //IF UP ARROW , 'A'
                    if(RXChar == 0x41){                     
                        downArrowOnce = false;
                        //IF VECTOR IS EMPTY, THEN BEEP
                        if(myVector.empty()){
                            write(STDOUT_FILENO, soundString.c_str(),1);
                        }
                        else{

                            //IF YOU'RE NOT AT THE END OF THE VECTOR
                            //cout << "up: " << upArrowOnce << endl;
                            if(it2 == myVector.begin() && firstTimeVisitingBegin){
                                firstTimeVisitingBegin = false;
                                clearSTDOUT(currentLineSize, currentLine);
                                write(STDOUT_FILENO, it2->c_str(), it2->size());
                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());  
                            }
                            else if( (it2 != myVector.end() - 1) ){

                                ++it2;
                                //cout << " our it2S tring is :" << it2->c_str() << endl;
                                if(it2 == myVector.end() - 1){
                                    //cout << "yeet" << endl;
                                    upArrowOnce = true;
                                }
                                clearSTDOUT(currentLineSize, currentLine);
                                //cout << "current line size is: " << currentLineSize << endl;
                               // cout << "CURRENT LINE IS: " << currentLine << endl;

                                
                                write(STDOUT_FILENO, it2->c_str(), it2->size());


                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());
                                
                            }//THEN GO TO THE NEXT
                            //ELSE IF YOU'VE DISPLAYED THE END ALREADY, THEN BEEP
                            else if(upArrowOnce == true){
                                write(STDOUT_FILENO, soundString.c_str(),1);
                                //cout << " ayy lmao" << endl;
                            }
                        }
                    }
                    //IF DOWN ARROW 'B'
                    else if(RXChar == 0x42){ // 'B' down arrow
                        upArrowOnce = false;


                        if(myVector.empty()){
                            write(STDOUT_FILENO, soundString.c_str(),1);
                        }
                        else{
                            clearSTDOUT(currentLineSize, currentLine);
                            //IF ITERATOR NOT AT BEGINNING OF VECTOR
                            if(it2 != myVector.begin()){
                                it2--;
                                write(STDOUT_FILENO, it2->c_str(), it2->size());
                                //cout << "yee " << endl;
                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());
                                

                            }
                            else if(downArrowOnce == true){
                                write(STDOUT_FILENO, soundString.c_str(),1);

                            }
                            else{
                                downArrowOnce = true;  
                                firstTimeVisitingBegin = true;                            
                            }//else if at beginnign write onto screen

                        //printf("It was a down arrow\n");
                        }
                    }
                    //OTHERWISE ITS SOMETHING ELSE
                    else
                        printf("False alarm\n");

                }
            }

            //IF WE PRESSED ENTER AND THERE IS SOMETHING IN CURRENT LINE
            if(0x0a == RXChar && currentLineSize != 0){
                firstTimeVisitingBegin = true;
                upArrowOnce = false;
                downArrowOnce = false;
                historyCounter++;


                //cout << "current line size is " << currentLineSize << endl;
                //IF WE HAVE MORE THAN 10 IN OUR HISTORY
                //cout << "history counter is " << historyCounter << endl;
                if(historyCounter > 10){
                    myVector.pop_back();
                    historyCounter--;
                }//DELETE ONE IN OUR HISTORY, AND DECREMENT HISTORY COUNTER

                //REMOVES NEWLINE AT THE END
                currentLine[currentLineSize] = 0;
                currentLineSize = 0;

                //INSERTS CURRENTLINE INTO VECTOR
                string str(currentLine);
                myVector.insert(myVector.begin(), str);
                //CALL PARSING FUNCTION
                parseCommand(str);

                it2 = myVector.begin();
                numPrevCommands++;

                write(STDOUT_FILENO, "\n", 1);
                printCurrentDir();


            }
            //ELSE IF WE PRESS ENTER WITH EMPTY LINE
            else if(0x0a == RXChar ){
                write(STDOUT_FILENO, "\n", 1);
                printCurrentDir();
            }
        }
    }
    
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}
