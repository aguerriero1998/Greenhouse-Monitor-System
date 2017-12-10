#include "farmer.h"

Ticker sensor_reader;

TMP36 temp(TEMP_IN); //using TMP36 library 
AnalogIn moisture(MOISTURE_IN);

DigitalOut relay(RELAY_OUT); 
DigitalOut fan(FAN_OUT);
DigitalOut pump(PUMP_OUT);
DigitalOut manual_led(MANUAL_LED);

InterruptIn manual_relay(MANUAL_RELAY);
InterruptIn manual_fan(MANUAL_FAN);
InterruptIn manual_pump(MANUAL_PUMP);
InterruptIn manual_enable(MANUAL_ENABLE);

bool manual_enabled; //if manual mode is enabled

float poll_time; //polling variables and thresholds
float relay_threshold;
float fan_threshold;
float pump_threshold;

int main() {
    //initialize variables
    relay = 1; //relay 1 == off 0 == on
    fan = 0;
    pump = 0;
    manual_led = 0;
    
    fan_threshold = 25; //25 degrees celsius
    relay_threshold = 16; //16 degrees celsius
    pump_threshold = .45; //45% moisture
    
    poll_time = 20; //20 seconds
    
    manual_enabled = false;
    
    manual_enable.rise(&toggle_manual);
    sensor_reader.attach(&read_sensors, poll_time); // for checking sensors at constant time interval
    
    printf("\n\rSystem Started\n\r");
    print_params();
   
    int choice;
    char term;
    int result;
    
    //run command line interface in infinite loop, interrupts handle everything else
    while(1) {
        
        print_menu();
        
        choice = 0;
        term = 0;
        result = 0;

        result = scanf("%d%c",&choice, &term);
        
        if(result != 2 || isalnum(term)){ //error checking
            printf("That is not a valid input\n\r");   
            continue; 
        }else{
            switch(choice){
                
                float val; // for reading 
                
                case 1:
                    print_sensors();
                    break;
            
                case 2:
                    //change poll time
                    printf("\n\rEnter a new poll time in seconds: \n\r");
                    result = scanf("%f%c", &val, &term);
                    if(result != 2 || isalnum(term)  || val <= 0){
                        printf("That is not a valid input\n\r");   
                         
                    }else{
                        poll_time = val;
                        
                        //only restarts polling if manual is false, if manual is true the 
                        //interrupt to turn off manual will restart the polling correctly
                        if(!manual_enabled) sensor_reader.attach(&read_sensors,poll_time); 
                    }
                    
                    break;
                    
                case 3: 
                    //change fan threshold
                    printf("\n\rEnter a new fan threshold in degrees celsius: \n\r");
                    result = scanf("%f%c", &val, &term);
                    if(result != 2 || isalnum(term)){
                        printf("That is not a valid input\n\r");   
                         
                    }else{
                        fan_threshold = val;
                    }
                    break;
                
                case 4: 
                    //change relay threshold
                    printf("\n\rEnter a new relay threshold in degrees celsius: \n\r");
                    result = scanf("%f%c", &val, &term);
                    if(result != 2 || isalnum(term)){
                        printf("That is not a valid input\n\r");   
                         
                    }else{
                        relay_threshold = val;
                    }
                    break;
                    
                case 5: 
                    //change pump threshold
                    printf("\n\rEnter a new pump threshold as a real number between 0 and 1: \n\r");
                    result = scanf("%f%c", &val, &term);
                    if(result != 2 || isalnum(term) || val < 0 || val > 1){
                        printf("That is not a valid input\n\r");   
                         
                    }else{
                        pump_threshold = val;
                    }
                    break;
                case 6:
                    print_params();
                    break;
                default:
                    printf("Please enter a number between 1 and 5\n\r"); 
                    break;
            }  
        }
    }
}

void print_menu(){
    printf("\n\rWelcome to the main menu for the automatic greenhouse\n\r");
    printf("1) Read Sensor Inputs\n\r");
    printf("2) Change Poll Time\n\r");
    printf("3) Change Fan Threshold\n\r");
    printf("4) Change Relay Threshold\n\r");
    printf("5) Change Pump Threshold\n\r"); 
    printf("6) Print Current Thresholds and Poll Time\n\r");  
} 

void print_sensors(){
    float temp_celcius;
    float moisture_val;
    
    temp_celcius = temp.read();
    moisture_val = moisture.read();
    
    printf("\n\rStart Sensor Reading\n\r");
    printf("\n\rDegrees Celsius %f\n\r", temp_celcius);
    printf("\n\rMoisture Value %f\n\r",moisture_val);
       
    
}

void print_params(){
    printf("\n\rFan Threshold: %f degrees celcius\n\r", fan_threshold);
    printf("\n\rRelay Threshold: %f degrees celcius\n\r", relay_threshold);
    printf("\n\rPump Threshold: %f%% moisture content\n\r", pump_threshold * 100);
    printf("\n\rPoll Time: %f seconds\n\r", poll_time);
    
}

void read_sensors(){
    
    float temp_celcius;
    float moisture_val;
    
    temp_celcius = temp.read();
    moisture_val = moisture.read();
    
    if(temp_celcius <= relay_threshold){ //if temp less than relay threshold 
        relay = 0; //relay on (0 == on)
    }else{
        relay = 1;  //relay off (1 == off)
    }
    
    if(temp_celcius >= fan_threshold){ //if temp greater than fan threshold
        fan = 1;    //fan on
    }else{
        fan = 0;    //fan off
    }
        
    if(moisture_val <= pump_threshold){ //if moisture reading less than threshold
        //turn pump on for short interval to avoid flooding the plant
        pump = 1;   
        wait(2);
        pump = 0;
    }else{
        pump = 0;    
    }
    
}

void toggle_fan(){
    //turning fan on with interrupt
    fan = !fan; 
    
    manual_fan.rise(NULL); //get rid of button bounce
    wait(WAIT_TIME);
    manual_fan.rise(&toggle_fan);
    

   
  
}

void toggle_relay(){
    //turning relay on with interrupt
    relay = !relay;
    
    manual_relay.rise(NULL); //get rid of button bounce
    
    wait(WAIT_TIME);
    manual_relay.rise(&toggle_relay);
    
   

}

void toggle_pump(){
    //turning pump on with interrupt
    pump = !pump;
    
    manual_pump.rise(NULL); //get rid of button bounce
    wait(WAIT_TIME);
    manual_pump.rise(&toggle_pump);

   

}
    
void toggle_manual(){
    //turning manual on with interrupt
    
    manual_enable.rise(NULL);
    
    if(manual_enabled){ //turn manual off
        manual_relay.rise(NULL); //disable interrupts
        manual_fan.rise(NULL);
        manual_pump.rise(NULL);  
        manual_led = 0; //turn led off
        manual_enabled = false; //change variable
        sensor_reader.attach(&read_sensors, poll_time); //restart polling
        
    }else{
        sensor_reader.detach(); //disable polling
        manual_relay.rise(&toggle_relay); //enable interrupts
        manual_fan.rise(&toggle_fan);
        manual_pump.rise(&toggle_pump);  
        manual_led = 1; //led on
        manual_enabled = true; //change variable
        
    }
    
    wait(WAIT_TIME); //debounce button
    
    
    manual_enable.rise(&toggle_manual);
}


