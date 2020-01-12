/*
 * Copyright (c) 2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "common_functions.h"
#include "UDPSocket.h"
#include "CellularLog.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TM1637.h"

// EYEWINK TM1637 6 Digit display with 5 keys test
#include "Font_7Seg.h"


#define UDP 0
#define TCP 1

// Number of retries /
#define RETRY_COUNT 3

// DisplayData_t size is 6 bytes (6 Grids @ 8 Segments) 
TM1637::DisplayData_t all_str   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  
TM1637::DisplayData_t cls_str   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  
TM1637::DisplayData_t hello_str1 = {0x00, C7_H, C7_E, C7_I, 0x00, 0x00};
TM1637::DisplayData_t hello_str2 = {C7_I, C7_O, 0x00, 0x00, 0x00, 0x00};

TM1637::DisplayData_t start_Test1= {0x00, C7_P, C7_H, 0x00, 0x00, 0x00};
TM1637::DisplayData_t start_Test2 = {0x00, C7_N, C7_8, 0x00, 0x00, 0x00};

TM1637::DisplayData_t Test_str1 = {C7_9, C7_6, C7_5, C7_7, 0x00, 0x00};
TM1637::DisplayData_t Test_str2 = {C7_4, C7_8, C7_9, C7_0, 0x00, 0x00};

char phon_nb;

// KeyData_t size is 1 bytes  
TM1637::KeyData_t keydata; 

//TM1637_EYEWINK declaration
//New Constructor
TM1637_EYEWINK EYEWINK1(D12, D13);       // MISO/SCK
TM1637_EYEWINK EYEWINK2(A3, PD_1);       // MISO/SCK
DigitalIn PIR_sensor(D3, PullUp); //PIR_sensor

char bits;

float speed = 0.0;                   // How speed
float speedAmount = 0.02;            // How many points to speed amount
int car_speed = 0;

int num = 0;

int Date_Sent = 0;
char Pon_NB[10]= {};


void BG96_Modem_PowerON(void)
{
    DigitalOut BG96_RESET(D7);
    DigitalOut BG96_PWRKEY(D9);
 
    BG96_RESET = 1;
    BG96_PWRKEY = 1;
    wait_ms(200);
 
    BG96_RESET = 0;
    BG96_PWRKEY = 0;
    wait_ms(300);
 
    BG96_RESET = 1;
    wait_ms(5000);
}

NetworkInterface *iface;

// Echo server hostname
const char *host_name = MBED_CONF_APP_ECHO_SERVER_HOSTNAME;

// Echo server port (same for TCP and UDP)
const int port = MBED_CONF_APP_ECHO_SERVER_PORT;

static rtos::Mutex trace_mutex;

#if MBED_CONF_MBED_TRACE_ENABLE
static void trace_wait()
{
    trace_mutex.lock();
}

static void trace_release()
{
    trace_mutex.unlock();
}

static char time_st[50];

static char* trace_time(size_t ss)
{
    snprintf(time_st, 49, "[%08llums]", Kernel::get_ms_count());
    return time_st;
}

static void trace_open()
{
    mbed_trace_init();
    mbed_trace_prefix_function_set( &trace_time );

    mbed_trace_mutex_wait_function_set(trace_wait);
    mbed_trace_mutex_release_function_set(trace_release);

    mbed_cellular_trace::mutex_wait_function_set(trace_wait);
    mbed_cellular_trace::mutex_release_function_set(trace_release);
}

static void trace_close()
{
    mbed_cellular_trace::mutex_wait_function_set(NULL);
    mbed_cellular_trace::mutex_release_function_set(NULL);

    mbed_trace_free();
}
#endif // #if MBED_CONF_MBED_TRACE_ENABLE

Thread dot_thread(osPriorityNormal, 512);

void print_function(const char *format, ...)
{
    trace_mutex.lock();
    va_list arglist;
    va_start( arglist, format );
    vprintf(format, arglist);
    va_end( arglist );
    trace_mutex.unlock();
}

void dot_event()
{
    while (true) {
        ThisThread::sleep_for(4000);
        if (iface && iface->get_connection_status() == NSAPI_STATUS_GLOBAL_UP) {
            break;
        } else {
            trace_mutex.lock();
            printf(".");
            fflush(stdout);
            trace_mutex.unlock();
        }
    }
}

/**
 * Connects to the Cellular Network
 */
nsapi_error_t do_connect()
{
    nsapi_error_t retcode = NSAPI_ERROR_OK;
    uint8_t retry_counter = 0;

    while (iface->get_connection_status() != NSAPI_STATUS_GLOBAL_UP) {
        retcode = iface->connect();
        if (retcode == NSAPI_ERROR_AUTH_FAILURE) {
            print_function("\n\nAuthentication Failure. Exiting application\n");
        } else if (retcode == NSAPI_ERROR_OK) {
            print_function("\n\nConnection Established.\n");
        } else if (retry_counter > RETRY_COUNT) {
            print_function("\n\nFatal connection failure: %d\n", retcode);
        } else {
            print_function("\n\nCouldn't connect: %d, will retry\n", retcode);
            retry_counter++;
            continue;
        }
        break;
    }
    return retcode;
}

/**
 * Opens a UDP or a TCP socket with the given echo server and performs an echo
 * transaction retrieving current.
 */
nsapi_error_t test_send_recv()
{
    nsapi_size_or_error_t retcode;
#if MBED_CONF_APP_SOCK_TYPE == TCP
    TCPSocket sock;
#else
    UDPSocket sock;
#endif

    retcode = sock.open(iface);
    if (retcode != NSAPI_ERROR_OK) {
#if MBED_CONF_APP_SOCK_TYPE == TCP
        print_function("TCPSocket.open() fails, code: %d\n", retcode);
#else
        print_function("UDPSocket.open() fails, code: %d\n", retcode);
#endif
        return -1;
    }

    SocketAddress sock_addr;
    retcode = iface->gethostbyname(host_name, &sock_addr);
    if (retcode != NSAPI_ERROR_OK) {
        print_function("Couldn't resolve remote host: %s, code: %d\n", host_name, retcode);
        return -1;
    }

    sock_addr.set_port(port);

    sock.set_timeout(15);
    int n = 0;
    char echo_string[100] = "GET /carnumberall.php HTTP/1.1\r\n"; 
    print_function(" %s\n", echo_string);

    char recv_buf[1000]={0, };
    char A[4]={0, };
#if MBED_CONF_APP_SOCK_TYPE == TCP
    retcode = sock.connect(sock_addr);
 
    if (retcode < 0) {
        print_function("TCPSocket.connect() fails, code: %d\n", retcode);
        return -1;
    } else {
        print_function("TCP: connected with %s server\n", host_name);
    }
    //retcode = sock.send((void*) echo_string, strlen(echo_string));
    retcode = sock.send((void*) echo_string, strlen(echo_string));
    if (retcode < 0) {
        print_function("TCPSocket.send() fails, code: %d\n", retcode);
        return -1;
    } else {
        print_function("TCP: Sent %d Bytes to %s\n", retcode, host_name);
    }

    n = sock.recv((void*) recv_buf, sizeof(recv_buf));
    print_function("dataRecv : %s\r\n",  recv_buf);

    int i=0;
    
    for(i=0;i<=1000;i++)
    {
        if( (recv_buf[i]=='C') && (recv_buf[i+1]=='0') && (recv_buf[i+2]=='1') && (recv_buf[i+3]=='0') && (recv_buf[i+4]=='3'))
        {
           
            Pon_NB[0] = recv_buf[i+11] ;
            Pon_NB[1] = recv_buf[i+12] ;
            Pon_NB[2] = recv_buf[i+13] ;
            Pon_NB[3] = recv_buf[i+14] ;
            Pon_NB[4] = recv_buf[i+15] ;
            Pon_NB[5] = recv_buf[i+16] ;
            Pon_NB[6] = recv_buf[i+17] ;
            Pon_NB[7] = recv_buf[i+18] ;
           
           

            Date_Sent=8;
        }
    }   
    
        


#else

    retcode = sock.sendto(sock_addr, (void*) echo_string, sizeof(echo_string));
    if (retcode < 0) {
        print_function("UDPSocket.sendto() fails, code: %d\n", retcode);
        return -1;
    } else {
        print_function("UDP: Sent %d Bytes to %s\n", retcode, host_name);
    }

    n = sock.recvfrom(&sock_addr, (void*) recv_buf, sizeof(recv_buf));
    for (int i=0; i<4; i++)
        print_function("%x\n",recv_buf[i]);
#endif

    sock.close();

    if (n > 0) {
        print_function("Received from echo server %d Bytes\n", n);
        return 0;
    }

    return -1;
}

void send_date(){
    print_function("\n\nmbed-os-example-cellular\n");
    print_function("\n\nBuilt: %s, %s\n", __DATE__, __TIME__);
#ifdef MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN
    print_function("\n\n[MAIN], plmn: %s\n", (MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN ? MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN : "NULL"));
#endif

    print_function("Establishing connection\n");

    BG96_Modem_PowerON();
    print_function("M2Mnet(BG96) Power ON\n");
        
#if MBED_CONF_MBED_TRACE_ENABLE
    trace_open();
#else
    dot_thread.start(dot_event);
#endif // #if MBED_CONF_MBED_TRACE_ENABLE

    // sim pin, apn, credentials and possible plmn are taken atuomtically from json when using get_default_instance()
    iface = NetworkInterface::get_default_instance();
    MBED_ASSERT(iface);

    nsapi_error_t retcode = NSAPI_ERROR_NO_CONNECTION;

    /* Attempt to connect to a cellular network */
    if (do_connect() == NSAPI_ERROR_OK) {
        retcode = test_send_recv();
    }

    if (iface->disconnect() != NSAPI_ERROR_OK) {
        print_function("\n\n disconnect failed.\n\n");
    }

    if (retcode == NSAPI_ERROR_OK) {
        print_function("\n\nSuccess. Exiting \n\n");
    } else {
        print_function("\n\nFailure. Exiting \n\n");
    }

#if MBED_CONF_MBED_TRACE_ENABLE
    trace_close();
#else
    dot_thread.terminate();
#endif // #if MBED_CONF_MBED_TRACE_ENABLE


}


int main()
{
    void Test_clear();
    void Test_Wake_up();
    void Nb_Selection(char Nb);
    
    int s = 0;

    print_function("Hello World\r\n"); //    
    
    EYEWINK1.cls(); 
    EYEWINK2.cls(); 
    EYEWINK1.writeData(all_str);
    EYEWINK2.writeData(all_str);

    wait_ms(2000);
    Test_clear();//문자 초기화 및 종료

    wait_ms(1000);
    Test_Wake_up(); //7세그먼트 깨우기

    EYEWINK1.writeData(hello_str1); //1번째 문자 입력
    EYEWINK2.writeData(hello_str2); //2번째 문자 입력

    wait_ms(1000);
    Test_clear(); //문자 초기화 및 종료
    
    wait_ms(100);
    Test_Wake_up(); //7세그먼트 깨우기
    EYEWINK1.writeData(start_Test1); //1번째 문자 입력 
    EYEWINK2.writeData(start_Test2); //2번째 문자 입력
 
    wait_ms(1000);
    Test_clear();
    
    int Test =0;
    int i = 0;
    int count = 0;
    
    char Date[10];
    char Leading_seat[5]={};
    char Backseat[5]={};
    
    char Phone_Nb1[5]={};
    char Phone_Nb2[5]={};

    send_date();
    while(1){
    test_send_recv();

    for(s=0;s<=7;s++)
        {
        wait_ms(1000);
        }
        
        if(Date_Sent==8){
            for(i=0;i<=3;i++){
                Leading_seat[i] = Pon_NB[i]; //앞자리
                Backseat[i] = Pon_NB[i+4]; //뒤자리
                
                Nb_Selection(Leading_seat[i]); // 앞자리 세그먼트 설정
                Phone_Nb1[i]=phon_nb; // 앞자리 세그먼트 설정
        
                Nb_Selection(Backseat[i]); // 뒷자리 세그먼트 설정
                Phone_Nb2[i]=phon_nb; // 뒷자리 세그먼트 설정
                } 
            
            print_function("\r\nPhone NB : 010-%s-%s\r\n",Leading_seat,Backseat); // 입력 받은 전화번호
            
            for(i=0;i<=3;i++){
             Test_str1[i]=Phone_Nb1[i]; // 7세그먼트로 출력 준비_앞
             Test_str2[i]=Phone_Nb2[i]; // 7세그먼트로 출력 준비_뒤
             } 

            Test_Wake_up();         
            EYEWINK1.writeData(Test_str1);   
            EYEWINK2.writeData(Test_str2); 
        
        wait_ms(100);
        Test_clear();
        wait_ms(1000);
        Date_Sent=0;
        print_function("[ DB_DATE :  %c, %c, %c, %c, %c, %c, %c, %c, %c] \r\n",Pon_NB[0],Pon_NB[1],Pon_NB[2],Pon_NB[3],Pon_NB[4],Pon_NB[5],Pon_NB[6],Pon_NB[7]);
        }
        wait_ms(100);

        if (PIR_sensor){
         Test_Wake_up();         
         EYEWINK1.writeData(Test_str1);   
         EYEWINK2.writeData(Test_str2); 
         wait_ms(3000); // 입맞대로 (추후 설정 가능)
         Test_clear();
         wait_ms(1000);
         }
         wait_ms(100);

    }
    
}
// EOF
void Test_clear()
{
        EYEWINK1.writeData(cls_str);   
        EYEWINK2.writeData(cls_str); 
        wait_ms(100);
        EYEWINK1.setBrightness(TM1637_BRT0);    // 죽이는 것    
        EYEWINK2.setBrightness(TM1637_BRT0);    // 죽이는 것    
}

void Test_Wake_up()
{
        EYEWINK1.setBrightness(TM1637_BRT4);  
        EYEWINK2.setBrightness(TM1637_BRT4);
}

void Nb_Selection(char Nb)
{
        if(Nb == '0'){
           phon_nb = C7_0;}
           
       else if(Nb == '1'){
           phon_nb = C7_1;}
           
       else if(Nb == '2'){
           phon_nb = C7_2;}
           
       else if(Nb == '3'){
           phon_nb = C7_3;}
           
       else if(Nb == '4'){
           phon_nb = C7_4;}
           
       else if(Nb == '5'){
           phon_nb = C7_5;}
           
       else if(Nb == '6'){
           phon_nb = C7_6;}
           
       else if(Nb == '7'){
           phon_nb = C7_7;}
           
       else if(Nb == '8'){
           phon_nb = C7_8;}
           
       else if(Nb == '9'){
           phon_nb = C7_9;}
}
