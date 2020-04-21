void printNumberX(int v) {
   
    int ones;
    int tens;
    int hundreds;
    int thousands;
        
    boolean negative = false;  

    if(v < -9999 || v > 9999) 
       return;
       
    if(v < 0) {
        negative=true;
        v=v*-1;
    }
    
    ones=v%10;
    v=v/10;
    
    tens=v%10;
    v=v/10;
    
    hundreds=v;   
    v=v/10;

    thousands=v;
      
    if(negative) {
       //print character '-' in the leftmost column 
       g_lc.setChar(0,44,'-',false);
    }
    else {
       //print a blank in the sign column
       g_lc.setChar(0,4,' ',false);
    }
    //Now print the number digit by digit
    g_lc.setDigit(0,3,(byte)thousands,false);
    g_lc.setDigit(0,2,(byte)hundreds,false);
    g_lc.setDigit(0,1,(byte)tens,false);
    g_lc.setDigit(0,0,(byte)ones,false);
    return;
}

void printTwoDigits( int location, int ones, int tens )
{
    //Now print the number digit by digit
    
    //Serial.print( "tens=" );
    //Serial.print( tens );
    //Serial.print( ", ones=" );
    //Serial.println( ones );
    
    g_lc.setDigit(0,location,(byte)tens,false);
    g_lc.setDigit(0,location-1,(byte)ones,false);

    return;
}


void printDuration(uint32_t v) {
   
    int seconds;
    int minutes;
    int hours;

    long maxtick = 60 * 60 * 24;

    if(v < 0 || v > maxtick ) {
      v = v | maxtick;
    }
    
    
    seconds = v % 60;
    v = v / 60;
    
    minutes = v % 60;
    v = v / 60;
    
    hours = v;   

/*
    Serial.print ( "hours=" );
    Serial.print ( hours );
    Serial.print ( ", mins=" );
    Serial.print ( minutes );
    Serial.print ( ", secs=" );
    Serial.println ( seconds );    
*/
    printTwoDigits( 7, hours % 10, hours / 10);  
    printTwoDigits( 4, minutes % 10, minutes / 10 );  
    printTwoDigits( 1, seconds % 10, seconds / 10 );     


    return;
}


