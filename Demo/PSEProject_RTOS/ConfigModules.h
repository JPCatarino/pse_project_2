/** LOCAL MACROS **************************************************************/

#define PBCLOCK 40000000L


#define BUTTON_RE8_MASK         0x0100
#define BUTTON_RE9_MASK         0x0200
#define BUTTON_RD0_MASK         0x0001
#define LED_RD1_MASK            0x0002    
#define LDR_RB0_MASK            0x0001

// TIMER
#define PS_OC_VALUE             1000
#define PS_ADC_VALUE            15625

// LED
#define LED_SET()   PORTDSET = LED_RD1_MASK;
#define LED_CLR()   PORTDCLR = LED_RD1_MASK;
#define LED_INV()   PORTDINV = LED_RD1_MASK;

// FUNCTIONS
void ConfigIOpins(void);
void ConfigOC2(void);
void ConfigTimer(void);
void InitInterruptController(void);
void print_string(char* message);
void print_current_state(uint8_t currentState);
void print_help(void);