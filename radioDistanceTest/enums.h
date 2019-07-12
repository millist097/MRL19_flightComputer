enum launchStatus { PREBURN, BURN, POSTBURN, APOGEE, DESCENT, 
LANDED};



/* Bit Operation macros*/
#define sbi(b,n)( (b) |= (1<<(n)) )
#define cbi(b,n)( (b) &= (~(1<<(n))) )
#define fbi(b,n)( (b) ^= (1<<(n)) )
#define rbi(b,n)( (b) & (1<<(n)) )

#define set_flag(combo) sbi(combo)
#define clear_flag(combo) cbi(combo)
#define flag_is_set(combo)    rbi(combo)
#define flag_is_clear(combo)  (!rbi(combo))

volatile uint16_t flags;

#define FLAG_DATA_AVAILABLE (flags),(0)
#define FLAG_TRANSMIT_DATA (flags),(1)
#define FLAG_MESSAGE_RECIVED (flags),(2)
#define FLAG_ERROR (flags),(3)
#define FLAG_DATA_RECIVED (flags),(4)
#define FLAG_ERROR_RECIVED (flags),(5)
#define FLAG_COMMAND_RECIVED (flags),(6)
#define FLAG_ACKNOWLEDGE_RECIVED (flags),(7)
