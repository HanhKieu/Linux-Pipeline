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


void clearSTDOUT(int sizeOfString ){
	string deleteString("\b \b");
	
	for(int i = 0; i < sizeOfString ; i++){
		write(STDOUT_FILENO, deleteString.c_str(), 3);
	}



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
   	string soundString("\a");
   	int historyCounter = 0;
   	bool upArrowOnce = false;
   	bool downArrowOnce = false;
   	
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

        	    if(RXChar == 0x5b){ // '['
        		    read(STDIN_FILENO, &RXChar, 1);

        	        if(RXChar == 0x41){ // 'A' up arrow
        	        	
        	        	downArrowOnce = false;

        	        	if(myVector.empty()){
        	        		write(STDOUT_FILENO, soundString.c_str(),1);
        	        	}
        	        	else{

		    		     	if( (it2 != myVector.end() - 1) ){
		    		     		clearSTDOUT(it2->size());
		    		     		write(STDOUT_FILENO, it2->c_str(), it2->size());
		    		     		++it2;
		    		     	} //if you're not at the end icrement
		    		     	else if(upArrowOnce == true){
		    		     		write(STDOUT_FILENO, soundString.c_str(),1);
		    		     	}
		    		     	else{
		    		     		clearSTDOUT(it2->size());
		    		     		write(STDOUT_FILENO, it2->c_str(), it2->size());
		    		     		upArrowOnce = true;
		    		     	}//if this is the end
	    		     	}
                    }

        		    else if(RXChar == 0x42){ // 'B' down arrow

        		    	upArrowOnce = false;
        		    	if(myVector.empty()){
        		    		write(STDOUT_FILENO, soundString.c_str(),1);
        		    	}
        		    	else{
        		    		clearSTDOUT(it2->size());
	        		    	if(it2 != myVector.begin()){
	        		    		it2--;
	        		    		write(STDOUT_FILENO, it2->c_str(), it2->size());
	        		    	} //if your iterator is at the beginning of vector and you press down make it ding
	        		    	else if(downArrowOnce == true){
	        		    		write(STDOUT_FILENO, soundString.c_str(),1);
	        		    	}
	        		    	else{
	        		    		write(STDOUT_FILENO, soundString.c_str(),1);
	        		    		downArrowOnce = true;
	        		    		
	        		    	}//else write onto screen

        		        //printf("It was a down arrow\n");
        		    	}
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

               	historyCounter++;

	        	if(historyCounter > 10){
	        		myVector.pop_back();
	        		historyCounter--;
	        	}//only up to 10 in our vector

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
