#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;


void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
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



int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    char currentLine[10000];
    int currentLineIndex = 0;
    vector<string> myVector;
    vector<string>::iterator it; //iterator used for insertion
    vector<string>::iterator it2; //vector iterator
    int numPrevCommands = 0;
   	string deleteString("\b \b");

//    string

    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    
    while(1){
        read(STDIN_FILENO, &RXChar, 1);

        if(isprint(RXChar) || RXChar == 0x0A){
        	write(STDOUT_FILENO, &RXChar, 1);
			currentLine[currentLineIndex++] = RXChar;
		}


        if(0x04 == RXChar){ // C-d
            break;
        }
        else if(0x7F == RXChar){ //backspace
        	write(STDOUT_FILENO, deleteString.c_str(), 3);
        	if(currentLineIndex >= 1)
        		currentLineIndex--;
        	currentLine[currentLineIndex] = 0;
        }
        else{
    	    if(0x1b == RXChar){ // ' '`
                read(STDIN_FILENO, &RXChar, 1);
                currentLine[currentLineIndex++] = RXChar;


        	    if(RXChar == 0x5b){ // '['
        		    read(STDIN_FILENO, &RXChar, 1);
                    currentLine[currentLineIndex++] = RXChar;


        	        if(RXChar == 0x41 && !myVector.empty()){ // 'A'
        		     	//printf("It was an up arrow\n");

        		     	cout << *it2 << endl;
        		     	if(it2 != myVector.end() - 1)
        		     		++it2;
                    }

        		    else if(RXChar == 0x42){ // 'B'
        		    	if(it2 != myVector.begin())
        		    		it2--;

        		    	cout << *it2 << endl;
        		        //printf("It was a down arrow\n");
                    }

                    else if(RXChar == 0x43 || RXChar == 0x44) // 'C' or 'D'
                        printf("It was a left or right arrow\n");

                    else //anything else
                        printf("False alarm\n");

		         }
	        }
            if(0x0a == RXChar){ //pressed enter 
                //printf("Pressed enter\n");
                //printf("Length of that command %d\n", currentLineIndex-1);
                currentLine[currentLineIndex-1] = 0;

                string str(currentLine);
                //printf("%s\n", str.c_str());
                currentLineIndex = 0;
                it = myVector.begin();
                myVector.insert(it, str);
                it2 = myVector.begin();
                numPrevCommands++;
            }
        }
    }
    
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}
