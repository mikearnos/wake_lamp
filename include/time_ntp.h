#include <IPAddress.h>

//Time Zone Selection
//const int TZ_OFFSET = 4*3600;  //AST UTC-4
const int TZ_OFFSET = -(5*3600);  //EST UTC-5
//const int TZ_OFFSET = 6*3600;  //CST UTC-6
//const int TZ_OFFSET = 7*3600;  //MST UTC-7
//const int TZ_OFFSET = 8*3600;  //PST UTC-8
//const int TZ_OFFSET = 9*3600;  //AKST UTC-9
//const int TZ_OFFSET = 10*3600;  //HST UTC-10

int isDST(time_t);

void startUDP(void);
void stopUDP(void);
void sendNTPpacket(IPAddress&);
uint32_t getTime(void);
void udpLoop(void);
//int isDST(void);

inline int getSeconds(uint32_t);
inline int getMinutes(uint32_t);
inline int getHours(uint32_t);
