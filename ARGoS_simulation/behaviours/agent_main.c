/*-----------------------------------------------------------------------------------------------*/
/* This file provides functions need to be implemented for Kilobot in order to work on the       */
/*  Kilogrid to test symmetry breaking two options with self-sourced noise and social interaction*/
/*-----------------------------------------------------------------------------------------------*/

// macro if we are in sim or reality -> command out if on real robot
#define SIMULATION


/*-----------------------------------------------------------------------------------------------*/
/* Imports - depending on the platform one has different imports                                 */
/*-----------------------------------------------------------------------------------------------*/

#include "kilolib.h"
#include <kilolib.h>
#include <math.h>

#ifdef SIMULATION

#include <stdio.h>
#include <float.h>
#include "agent.h"
#include <debug.h>


#else

#include "utils.h"  // TODO check if this is needed ?!?
#include "kilob_tracking.h"
#include "kilo_rand_lib.h"
#include "../communication.h"
#include "kilob_messaging.h"


#endif


/*-----------------------------------------------------------------------------------------------*/
/* Define section here you can define values, e.g., messages types                               */
/*-----------------------------------------------------------------------------------------------*/
// options
#define UNCOMMITTED 99  //opinion for uncommitted
// message types
#define FROMBOT 9  //the message is from robot and not kilogrid
#define INIT_MSG 10  // initial message from the kilogrid
#define GRID_MSG 11  // info msg from the kilogrid with option and position
#define VIRTUAL_AGENT_MSG 12  // msg forwarded from the kilogrid
#define TO_KILOGRID_MSG 62

/*-----------------------------------------------------------------------------------------------*/
/* Change these when running experiment                                                          */
/*-----------------------------------------------------------------------------------------------*/
#define MODEL 1   // 0 --> Voter Model      1 --> CrossInhibition
double noise = 0.1; // SET THIS TO -1 FOR NO NOISE, 0.1--> 0.05, 0.5-->0.25
/*-----------------------------------------------------------------------------------------------*/

//opinion = A -->1   //opinion = B --> 2  //uncommited = C --> UNCOMITTED
int currentopinion; //1

double timer; // to hold time to be in each state
double avg_exploration_time = 3300.0; //***--> time to be in exploration state--> fixed
double avg_uncommitted_time = 200.0; // time to stay in dissemination for uncommitted agents
double dissemparam = 1300.0;

int foundmodules[18][38] = {0}; //to keep track of tiles visited in one exploration cycle
float qratio; //to store quality based ont the tiles explored


//int state = 0; //0--> Exploration , 1-->Dissemination , 2-->Voting // start in exploration state

/*-----------------------------------------------------------------------------------------------*/
/* Enum section - here we can define useful enums                                                */
/*-----------------------------------------------------------------------------------------------*/
typedef enum{
    false = 0,
    true = 1,
} bool;

/* Enum for different motion types */
typedef enum {
    STOP = 0,
    FORWARD,
    TURN_LEFT,
    TURN_RIGHT,
} motion_t;

/* Enum for different states */
typedef enum {
    EXPLORATION,
    DISSEMINATION,
    VoteOrNoise,
} state;
/*-----------------------------------------------------------------------------------------------*/
state current_state = EXPLORATION;

/*-----------------------------------------------------------------------------------------------*/
/* Motion related Variables                                                                      */
/*-----------------------------------------------------------------------------------------------*/
motion_t current_motion_type = STOP;
unsigned int turning_ticks = 0;
const uint8_t max_turning_ticks = 150; //*** constant to set maximum rotation to turn during random walk
const uint32_t max_straight_ticks = 300; //*** set the time to walk straight before randomly turning
//const uint32_t broadcast_ticks = 32;
uint32_t last_motion_ticks = 0;

uint8_t kilogrid_commitment = 0;  // This is the initial commitment attained from kilogrid
//float my_commitment_quality = 0.0;
int last_changed = 0;

//uint8_t communication_range = 0;  // communication range in cells

/*-----------------------------------------------------------------------------------------------*/
/* Communication variables - used for communication and stuff                                    */
/*-----------------------------------------------------------------------------------------------*/
bool broadcast_msg = false;
// how often we try to send the msg - in simulation once is sufficient
#ifdef SIMULATION
#define MSG_SEND_TRIES 1
#else
#define MSG_SEND_TRIES 10
#endif
// Kilobot -> Kilogrid
uint32_t msg_counter_sent = MSG_SEND_TRIES + 1;  // counts the messages sent
uint32_t msg_number_send = 0;  // change if you want to send a msg
uint32_t msg_number_current_send = 0;  // var for checking against the last
// Kilogrid -> Kilobot
bool init_flag = false;
bool received_grid_msg_flag = false;
bool received_virtual_agent_msg_flag = false;
// message content
#ifdef SIMULATION

//uint8_t communication_range_msg = 0;
//uint8_t x_pos_msg = 0;
//uint8_t y_pos_msg = 0;
//uint32_t msg_counter = 0;

#else
IR_message_t* message;
#endif

// Flag to keep track of new messages.
message_t message;
int new_message = 0;

int last_changed;
// Flag to keep track of message transmission.
int message_sent = 0;

int received_option; //to save an opinion received from another bot
int received_option_kilogrid; // to save option received from kilogrid
int received_uid; //previously used to save neighbours kilouid temporarily
/*-----------------------------------------------------------------------------------------------*/
/* Arena variables                                                                               */
/*-----------------------------------------------------------------------------------------------*/

bool hit_wall = false;  // set to true if wall detected
//to keep track of tiles and quality
int total_tiles_found;
int tiles_of_my_option;
int option_received_from_neighbour;

///NOT USED ANYMORE
//int neighbourid[SWARMSIZE] = {};//***change the size if running with more than 25 kilobots--> mention no of robots used in size
//int neighbouropinion[SWARMSIZE] = {}; //right now voter *** k->5, change to vary according to the majority rule

//sets the qratios, 6/3=2, change values to vary//  double q3 = 0.003;//***quality for A,// double q1 = 0.006;//*** quality for B
//double q3 = 0.003;//***qualities are same for A and B for now
//double q1 = 0.003;//***qualities are same for A and B for now

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------------------*/
/* Setting the Motion of the Bot                                                                 */
/*-----------------------------------------------------------------------------------------------*/
void set_motion( motion_t new_motion_type ) {
    if( current_motion_type != new_motion_type ){

        switch( new_motion_type ) {
            case FORWARD:
                spinup_motors();
                set_motors(kilo_straight_left,kilo_straight_right);
                break;
            case TURN_LEFT:
                spinup_motors();
                set_motors(kilo_turn_left,0);
                break;
            case TURN_RIGHT:
                spinup_motors();
                set_motors(0,kilo_turn_right);
                break;
            case STOP:
                set_motors(0,0);
                break;
        }
        current_motion_type = new_motion_type;
    }
}

/*-----------------------------------------------------------------------------------------------*/
/* Random Walk                                                                                   */
/*-----------------------------------------------------------------------------------------------*/
void random_walk(){
    switch( current_motion_type ) {
        case TURN_LEFT:
        case TURN_RIGHT:
            if(  kilo_ticks > last_motion_ticks + turning_ticks ) {
                /* start moving forward */
                last_motion_ticks = kilo_ticks;
                set_motion(FORWARD);
            }
            break;
        case FORWARD:
            //spinup_motors();
            //set_motors(20,20);
            if( kilo_ticks > last_motion_ticks + max_straight_ticks ) {
                /* perform a random turn */
                last_motion_ticks = kilo_ticks;
                if( rand()%2 ) {
                    set_motion(TURN_LEFT);
                    current_motion_type = TURN_LEFT;
                }
                else {
                    set_motion(TURN_RIGHT);
                    current_motion_type = TURN_RIGHT;
                }
                turning_ticks = rand()%max_turning_ticks + 1;
            }
            break;
        case STOP:
            set_motion(STOP);
        default:
            set_motion(FORWARD);
    }
}
/*-----------------------------------------------------------------------------------------------*/
/* Function to check if the robot is against the wall                                             */
/*-----------------------------------------------------------------------------------------------*/
void check_if_against_a_wall() {
    // when the hitwall flag is true -- (either at the border tiles or white buffer)
    if(hit_wall){
        if( rand()%2 ) {
            set_motion(TURN_LEFT);
            current_motion_type = TURN_LEFT;
            set_color(RGB(2, 0, 2));

            delay(5100);
            set_color(RGB(0, 0, 0));

            set_motion(FORWARD);
            delay(9000);

            random_walk();
        }else {
            set_motion(TURN_RIGHT);
            current_motion_type = TURN_RIGHT;

            set_color(RGB(2, 0, 2));
            delay(5100);
            set_color(RGB(0, 0, 0));
            set_motion(FORWARD);
            delay(9000);
            random_walk();
        }
        turning_ticks = rand()%max_turning_ticks + 1;

    }
}

/*-----------------------------------------------------------------------------------------------*/
/* Function to get Exponential Distribution for timing to stay is Dissem state                   */
/*-----------------------------------------------------------------------------------------------*/
double ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

/*-----------------------------------------------------------------------------------------------*/
/* Function to get a random 1 or 0 to check for self-sourced noise or social interaction         */
/*-----------------------------------------------------------------------------------------------*/
double r2()
{
    return (double)rand() / (double)RAND_MAX ;
}

/*-----------------------------------------------------------------------------------------------*/
/* DEPRECATED, not used: This function is used to count the number of occurrences for            */
/*               each opinion if majority model is used                                          */
/*-----------------------------------------------------------------------------------------------*/
/*
int countOccurrences(int arr[], int n, int x)
{
    int res = 0;
    for (int i=0; i<n; i++)
        if (x == arr[i])
          res++;
    return res;
}
 */


/*-----------------------------------------------------------------------------------------------*/
/* DEPRECATED, not used: This function is used to check if a received uid from kilobot is        */
/*  already present in the neighbour array. If present that means the kilobot has already voted  */
/*-----------------------------------------------------------------------------------------------*/

/*
int find_index(int a[], int num_elements, int value)
{
   int ii;
   for (ii=0; ii<num_elements; ii++)
   {
	 if (a[ii] == value)
	 {
	    return(value);  // it was found
	 }
   }
   return(-1);  //if it was not found
}
*/

/*-----------------------------------------------------------------------------------------------*/
/* This function is used to convert the tiles perceived in an expl-dissem cycle to quality       */
/* of  its opinion. If q>=0.5, all qr is taken as 1. The function is inverse lambda expon.       */
/*-----------------------------------------------------------------------------------------------*/
/*
void findqualityratio(){

    qratio = ((float)tiles_of_my_option/total_tiles_found);  //find % of tiles of opinion bot supports

    if(qratio >= 0.5){ //if % more than or equal to 0.5
        qratio = 1/(dissemparam); //like valentini model inverse lambda, 1300 instead of 1000 to increase dissem time

    }else{ //otherwise calculate dissem time based on % found 0-0.4999
        qratio = 1/(((float)tiles_of_my_option/total_tiles_found)*dissemparam); //like valentini model inverse lambda, 1300 instead of 1000 to increase dissem time

    }

    // qratio = 1/(((float)tiles_of_my_option/total_tiles_found)*1000); //like valentini model inverse lambda

    printf("%d tile my op %d total tiles and qr %f \n", tiles_of_my_option,total_tiles_found, qratio);

}
*/
/*-----------------------------------------------------------------------------------------------*/
/* This function is used to find the time to disseminate based on the output of qr from          */
/*                      findqualityratio() function.                                            */
/*-----------------------------------------------------------------------------------------------*/
/*
void calculatedissemtime(){

    if (isnan(qratio) || isinf(qratio)){  //if quality was found to be 0
        if(MODEL == 1 && currentopinion == UNCOMMITTED){ //if cross-inhibition and if bot is uncommitted

                printf("comes to time of uncommitted agent in noisy switch \n");
                timer =  ran_expo(quncommitted); //set time to be in dissem state but will not talk

        }else{ //if not uncommitted and tiles own opinion found to be = 0
            printf("% f  goes directly to voting    \n", qratio);
            timer = 0;  //no time as will go directly to poll/noisy switch state
            //timer = ran_expo(0.003);
        }


    }else{ //if quality was not 0

        //printf("% f  goes to random  \n", qratio);
        timer = ran_expo(qratio); //get the time to disseminate based on quality
        //}
    }
}
*/

void findqualityratio(){
    if (tiles_of_my_option > 0){
        qratio = ((float)tiles_of_my_option/total_tiles_found);
    } else {
        qratio = 0;
    }
    printf("%d tile my op %d total tiles and qr %f \n", tiles_of_my_option,total_tiles_found, qratio);


}

void calculatedissemtime(){

    if(MODEL == 1 && currentopinion == UNCOMMITTED){ // if MODEL is cross-inhibition and bot is uncommitted

        timer =  ran_expo(1.0/avg_uncommitted_time); //set time to be in dissem state but will not talk
        printf("timer is %f \n", 1.0/avg_uncommitted_time);


    } else {
        double lambda = 0;
        if(qratio >= 0.5){ //if % more than or equal to 0.5
            lambda = 1/(dissemparam); //like valentini model inverse lambda, 1300 instead of 1000 to increase dissem time

        }else { //otherwise calculate dissem time based on % found 0-0.4999
            lambda = 1 / ((qratio) * dissemparam); //like valentini model inverse lambda, 1300 instead of 1000 to increase dissem time
        }
        timer = ran_expo(lambda);

        printf("timer is %f \n", lambda);

    }


}
/*-----------------------------------------------------------------------------------------------*/
/*                              The Exploration function                                         */
/*                                                                                               */
/*-----------------------------------------------------------------------------------------------*/
void gotoexploration(){

    //  random_walk(); //start with random walk

    //set led colours
    if (currentopinion == 1){
        set_color(RGB(1, 0, 0));

    } else if (currentopinion == 2){
        set_color(RGB(0, 1, 1));

    }else{
        set_color(RGB(2, 2, 0)); //uncommitted

    }


    //if time for exploration not over yet, do nothing else move on to dissemination state
    if ((kilo_ticks - last_changed) < timer) {//check if still within time for exploration state or not
        check_if_against_a_wall(); //check if hitting the wall
    } else{ //if not in exploration state

        findqualityratio(); //find quality of own opinion based on tiles incurred in the exploration cycle


        // printf("%d tiles of my option and %d total tiles and actual is %f   ",tiles_of_my_option,total_tiles_found,percentage);
        // printf("%.5f no of percentage \n", (float)((1-((float)percentage))/100));


        calculatedissemtime(); //calculate the time that the bot should be disseminating based on quality found

        if(timer == 0){ //if 0 tiles found of same opinion
            printf("timer is 00 here \n");
            current_state = VoteOrNoise;//directly go to noisy switch or polling state
            // random_walk();
        }else{
            current_state = DISSEMINATION;//go to Dissemination mode
            // set_color(RGB(0, 0, 0));
        }

        last_changed = kilo_ticks;

        //reset the variable that are used to find the qr for next exploration-dissem cycle
        memset(foundmodules, 0, sizeof(foundmodules[0][0]) * 18 * 38);
        tiles_of_my_option = 0;
        total_tiles_found = 0;
        qratio = 0;
        // set_color(RGB(0, 0, 0));

    }

}

/*-----------------------------------------------------------------------------------------------*/
/*           The Noisy switch function- self sourcing noise from Kilogrid                        */
/*                                                                                               */
/*-----------------------------------------------------------------------------------------------*/

void donoisyswitch(){
    printf("opts for noisy switch noise fro kilgorid \n");
    check_if_against_a_wall(); //are we getting a wall signal from Kilogrid


    if(init_flag){  // initalization happened from Kilogrid
        // run logic
        // process received msgs
        if (received_grid_msg_flag) { //if received message from kilogrid
            printf("receives message from grid \n");


            if(kilogrid_commitment == 1 || kilogrid_commitment == 6){  //if it option A- red recived from Kilogrid  (1 for a normal red tile and 6 for border red tile)
                printf("%d  changes commitment to 1 from kilogrid\n", kilogrid_commitment);

                currentopinion = 1; //change opinion to A- Red - 1
                set_color(RGB(1, 0, 0));


            } else if(kilogrid_commitment == 3 || kilogrid_commitment == 9){  //if its option b- Blue received from Kilogrid (3 for a normal blue tile and 9 for border blue tile)
                printf("%d  changes commitment to 2 from kilogrid\n", kilogrid_commitment);

                currentopinion = 2; //change option to B- Blue - 2
                set_color(RGB(0, 1, 1));


            }


            received_grid_msg_flag = false;  //set Kilogrid messaged received to False to get next

        }

        if (received_virtual_agent_msg_flag) { //this we are not using its when Kilobot does communciation
            // update_virtual_agent_msg();
            received_virtual_agent_msg_flag = false;
        }

    }





    //set parameters for sending message to other bots
    message.data[1] = currentopinion;
    message.data[2] = kilo_uid;
    message.crc = message_crc(&message);

    timer =  ran_expo(1.0/avg_exploration_time); //get time for exploration
    current_state = EXPLORATION; //go to exploration state

    last_changed = kilo_ticks;
    set_color(RGB(0, 0, 0));




}

/*-----------------------------------------------------------------------------------------------*/
/*           The Dissemination Function                                                          */
/*                                                                                               */
/*-----------------------------------------------------------------------------------------------*/
void gotodissemination(){
    //printf("goes to dissemination \n");
    //printf("enter dissm %d  %d \n", tiles_of_my_option,total_tiles_found);

    //random_walk(); //lets random walk again
    check_if_against_a_wall();  //check if bot is not near or on wall

    if ((kilo_ticks - last_changed) < timer) { //if within dissemination time

        if(currentopinion != UNCOMMITTED){ //if the agent is not uncommitted
            broadcast_msg = true; //then send out message to other bots

            //set led colours
            if (currentopinion == 1){
                set_color(RGB(1, 0, 0));

            } else if (currentopinion == 2){
                set_color(RGB(0, 1, 1));

            }

        }else{ //if uncommitted

            set_color(RGB(3, 3, 0)); //just light yellow

        }

    }else{ //if time for dissem is over
        //check_if_against_a_wall();  //is the bot getting hit the wall message
        last_changed = kilo_ticks;
        current_state = VoteOrNoise;//go to polling or noisy switch state

    }
    // random_walk();



}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* NOT USED:Function to process the data received from the kilogrid regarding the environment    */
/*-----------------------------------------------------------------------------------------------*/
/*
void update_grid_msg() {
    // TODO add logic here
   // set_color(RGB(2, 2, 0));
  //  delay(2000);
    set_color(RGB(0, 0, 0));

    return;
}
*/
/*-----------------------------------------------------------------------------------------------*/
/*                          The Polling Function- Social interaction                             */
/*                                                                                               */
/*-----------------------------------------------------------------------------------------------*/
void poll(){

    if (new_message == 1){ //if bot seems to be getting a message from neighbour

        new_message = 0; //set message received flag to 0-not received
        option_received_from_neighbour = received_option; //consider the opinion of a message received from neighbour and save it


        if (MODEL == 1){ //if cross inhibition

            if(currentopinion != UNCOMMITTED){ //if I am not uncommited

                if (option_received_from_neighbour != currentopinion){ //check if my opinion is not equal
                    //go uncommited
                    printf("becomes uncommmitttedddddddddddd");
                    currentopinion = UNCOMMITTED;
                    //set_color(RGB(0, 3, 0));
                    //delay(1000);
                    //    delay(1000);


                }//else{//stay with my own opinion

                //  currentopinion = option_received_from_neighbour;

                // }

            }else{ //if I am uncommitted then get the opinion attained from neighbour

                currentopinion = option_received_from_neighbour;



            }


        }
        if (MODEL == 0){ //if voter model
            //printf("comeshere \n");
            currentopinion = option_received_from_neighbour;// just switch the option to opinion received from neighbour

        }

        message.data[1] = currentopinion;
        message.data[2] = kilo_uid;
        message.crc = message_crc(&message);

    }else{    //if no neighbour found, stay with current opinion

        //set messaege parameters
        //currentopinion = current_opinion;
        message.data[1] = currentopinion;
        message.data[2] = kilo_uid;
        message.crc = message_crc(&message);


    }


    option_received_from_neighbour = 0; //reset any option received from neighbour
    //go to exploration state
    current_state = EXPLORATION;
    timer =  ran_expo(1.0/avg_exploration_time); // get the time for exploration
    last_changed = kilo_ticks;
    set_color(RGB(0, 0, 0));

}



/*-----------------------------------------------------------------------------------------------*/
/* NOT USED: Function to process the data received from the kilogrid regarding other robots      */
/*-----------------------------------------------------------------------------------------------*/
/*
void update_virtual_agent_msg() {
    // TODO add logic here
    return;
}

*/


/*-----------------------------------------------------------------------------------------------*/
/* This function implements the callback, for when the robot receives an infrared message (here  */
/* also from the kilogrid and from other robots)                                                 */
/*-----------------------------------------------------------------------------------------------*/
// because there has been an "updated" version of the kilo_lib we have a slightly different
// implementation
#ifdef SIMULATION
void message_rx( message_t *msg, distance_measurement_t *d ) {
#else
    void message_rx( IR_message_t *msg, distance_measurement_t *d ) {
#endif
    // check the messages

    if(msg->type == FROMBOT){ //if message from another bot

        // printf("hey i am a bot");

        new_message = 1;        // Set the flag on message reception.
        received_option = msg->data[1]; //get its option
        received_uid = msg->data[2]; //get its uid
    }
    if(msg->type == GRID_MSG){ //if message from Kiogrid
        // printf("message from grid\n"); // printf("%hhu\n", msg->data[0]); // printf("%hhu\n",msg->data[1]);

        received_option_kilogrid = msg->data[2];// get the opinion of the tile
        received_grid_msg_flag = true; //set the flag that message received from Kilogrid to true


        if (current_state == EXPLORATION){ //if the bot is in exploration state
            if(received_option_kilogrid != 0){ //if the the bot is not on the wall-white border
                if (foundmodules[msg->data[0]][msg->data[1]] == 0){ //if tile not counted previously

                    foundmodules[msg->data[0]][msg->data[1]] = 1; //set the flag that tile has been counted now
                    total_tiles_found += 1;
                    if((received_option_kilogrid==1 || received_option_kilogrid==6) && currentopinion == 1){ //if I am Red and I receive red from kilogrid
                        tiles_of_my_option +=1 ;

                    }else if((received_option_kilogrid==3 || received_option_kilogrid==9) && currentopinion == 2){ //if I am blue and I receive blue from Kilogrid
                        tiles_of_my_option +=1 ;

                    }
                }
            }
        }

        // printf("%hhu\n",msg->data[2]);
        // printf("%hhu\n",msg->data[3]);
        if (received_option_kilogrid == 6 || received_option_kilogrid == 9 || received_option_kilogrid == 0){  // robot sensed wall or near wall
            //  printf("received hitwall option");
            hit_wall = true; //-> set hit wall flag to true
            if(received_option_kilogrid == 6 || received_option_kilogrid == 9){ //if near the border of wall and not on white wall

                kilogrid_commitment = msg->data[2]; //still get the opinion from grid
                //printf("gets commitments\n");

            }
        }else{
            hit_wall = false; //not hitting the wall
            kilogrid_commitment = msg->data[2]; //get the opinion from kilogrid
        }

    }
    if(msg->type == INIT_MSG && !init_flag){
        // TODO add logic ...
        // example usage
//        kilogrid_commitment = msg->data[0];
//        my_commitment_quality = msg->data[1];
//        NUMBER_OF_OPTIONS = msg->data[2];
//        option_to_sample = rand() % NUMBER_OF_OPTIONS;
//        current_ground = msg->data[3];
//        communication_range = msg->data[4];

        init_flag = true;
    }else if(msg->type == GRID_MSG && init_flag){
        // TODO add logic ...
        //received_grid_msg_flag = true;
    }else if(msg->type == VIRTUAL_AGENT_MSG  && init_flag){
        // TODO add logic ...
        //received_virtual_agent_msg_flag = true;
    }
    return;
}


/*-----------------------------------------------------------------------------------------------*/
/* NOT USED: This function implements the sending to the kilogrid. you should call this function */
/* everyloop cycle because in reality you dont have a indicator if the message was received so we*/
/* have to send it multiple times. The when and how often to send a message should be            */
/* implemented here!                                                                             */
/*-----------------------------------------------------------------------------------------------
void message_tx(){

    // implementation differs because in simulation we use the debugstruct - faster and easier to
    // understand
    // in reality we send infrared msg - we send more than one to make sure that the messages arrive!
    if (msg_number_current_send != msg_number_send){
        msg_number_current_send = msg_number_send;
        msg_counter_sent = 0;
    }
#ifdef SIMULATION
    // reset broadcast flag - needed in simulation to stop sending messages
    if(msg_counter_sent == MSG_SEND_TRIES){
        debug_info_set(broadcast_flag, 0);
        msg_counter_sent += 1;
    }
#endif
    // send msg if not sent enough yet
    if (msg_counter_sent < MSG_SEND_TRIES){
#ifdef SIMULATION
        // count messages
        msg_counter_sent += 1;
#else
        if((message = kilob_message_send()) != NULL) {
            msg_counter_sent += 1;

        }
#endif
    }
}
*/
void message_tx_success(){ //if transmitted
    broadcast_msg = false; //set transmitted flag to false
    //set the colour
    if (currentopinion == 1){
        set_color(RGB(2, 0, 0));
        delay(10);
        set_color(RGB(0, 0, 0));
    } else if (currentopinion == 2){
        set_color(RGB(0, 2, 2));
        delay(10);
        set_color(RGB(0, 0, 0));
    }

}
message_t *message_tx()
{
    if( broadcast_msg ) { //if broadcast message flag is set to true (only in dissem state function)
        return &message;
    }
    return 0;
}

/*-----------------------------------------------------------------------------------------------*/
/* NOT USED: Setting values of the message                                                                 */
/*-----------------------------------------------------------------------------------------------*/
void set_message(){
    // TODO this needs to be adjusted on what kind of messages you want to send: fill in data !
#ifdef SIMULATION
    msg_number_send += 1;
    debug_info_set(broadcast_flag, 1);
    debug_info_set(type, MSG_T_VIRTUAL_ROBOT_MSG);
    debug_info_set(data0, 1);
    debug_info_set(data1, 2);
    debug_info_set(data2, 3);
    debug_info_set(data3, 4);
    debug_info_set(data4, msg_number_send);
    debug_info_set(data5, 6);
    debug_info_set(data6, 7);
    debug_info_set(data7, 8);
#else
    // sample usage
    /*
    message->type = TO_KILOGRID_MSG;
    message->data[0] = kilogrid_commitment;
    message->data[1] = communication_range;
    message->data[2] = robot_gps_x;
    message->data[3] = robot_gps_y;
    message->data[4] = msg_number_current_send;
    */
#endif
}


/*-----------------------------------------------------------------------------------------------*/
/* Init function                                                                                 */
/*-----------------------------------------------------------------------------------------------*/
void setup(){


    srand(rand_hard());

    random_walk();

    //random timing for motion
    last_motion_ticks = rand() % max_straight_ticks + 1;

    //save the current ticks for comparison later on
    last_changed = kilo_ticks;
    message.type = FROMBOT; // set I am a bot
    // Quality A=1, B=2

    if(kilo_uid % 2 == 0){ //choose muy opinion based on odd or even kilouid
        currentopinion = 1;
    }else{
        currentopinion = 2;
    }

    // set parameters fro dissemination
    message.data[0] = currentopinion;
    //Opinion A=1 , B=2, U =3
    //red
    message.data[1] = currentopinion;
    message.data[2] = kilo_uid;
    message.crc = message_crc(&message);
    timer =  ran_expo(1/avg_exploration_time); //get time to be in exploration state



    //kilogrid variables
#ifndef SIMULATION
    // for tracking the robot in real life
    kilob_tracking_init();
    kilob_messaging_init();
    tracking_data.byte[0] = kilo_uid;
    tracking_data.byte[5] = 0;
#endif

}


/*-----------------------------------------------------------------------------------------------*/
/* Main loop                                                                                     */
/*-----------------------------------------------------------------------------------------------*/
void loop() {

    if(init_flag){  // initialization happened and messaged received from Kilogrid

        // if (received_grid_msg_flag) {
        //random_walk();
        //update_grid_msg();
        // check_if_against_a_wall();  // checks if the robot is on wall
        // received_grid_msg_flag = false;
        // }

        if (received_virtual_agent_msg_flag) {
            // update_virtual_agent_msg();
            received_virtual_agent_msg_flag = false;
        }

    }

    check_if_against_a_wall();

    random_walk();

    if (current_state == EXPLORATION){ // if state is set to 0


        gotoexploration(); //go to exploration

    }



    if (current_state == VoteOrNoise){  // state is set to choose between Vote or Noise

        //get the random number 0-1 to flip between self-sourced or social
        double u = r2();

        if (u <= noise){ //switch to noise check
            set_color(RGB(2, 2, 0));

            donoisyswitch(); //do noisy switch

        } else{ //Go to Voting

            poll();

        }

    }
    if(current_state == DISSEMINATION ){ // state is set to 1- Dissem

        gotodissemination(); //go to dissemination function



    }



#ifdef SIMULATION
    // debug info - is now also important for the inter-robot communication, so do not delete
    debug_info_set(currentopinion, currentopinion);
#else
    tracking_data.byte[1] = received_x;
        tracking_data.byte[2] = received_y;
        tracking_data.byte[3] = com_range;
        tracking_data.byte[4] = msg_number_current;
        kilob_tracking(&tracking_data);
#endif
}


/*-----------------------------------------------------------------------------------------------*/
/* Main function - obviously needs to be implemented by both platforms.                          */
/*-----------------------------------------------------------------------------------------------*/
int main(){
    // initialize the hardware of the robot
    kilo_init();
    // now initialize specific things only needed for one platform
#ifdef SIMULATION
    // create debug struct - mimics the communication with the kilogrid
    debug_info_create();
#else
    utils_init();
#endif
    // callback for received messages
    kilo_message_rx = message_rx;
    // start control loop

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;
    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;
    kilo_start(setup, loop);
    return 0;
}
