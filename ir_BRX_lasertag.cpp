
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                          BBBB     RRRR     X    X
//                          B   B    R   R     X  X
//                          BBBB     RRRR       XX
//                          B   B    R   R     X  X
//                          BBBB     R    R   X    X
//==============================================================================

#define BITS                          25    // The number of bits in the command
                                    // 4    = Bullet Type
                                    // 6    = Player ID
                                    // 2    = TeamID
                                    // 8    = Damage
                                    // 3    = Unknown
                                    // 2    = Parity

#define HDR_MARK                    2000	// The length of the Header:Mark
#define HDR_SPACE                    500	// The length of the Header:Space

#define ZERO_BIT                     500	// The length of a Bit:Mark for 0's
#define ONE_BIT                     1000	// The length of a Bit:Mark for 1's
#define BIT_SPACE                    500	// The length of a Bit:Space
#define LONG_PAUSE                  5000	// The length between packets.

//#define TYPE_LAZERTAG_TAG           100        // Decode this as a tag
//#define TYPE_LAZERTAG_BEACON        101        // Decode this a a beacon
//
//#define LAZERTAG_SIG_PS             3000    // Presync: Active 3ms +/- 10%
//#define LAZERTAG_SIG_PSP            6000    // Presync Pause: Inactive 6ms +/- 10%
//#define LAZERTAG_SIG_TAG_SYNC       3000    // Sync: Active 3ms +/- 10%
//#define LAZERTAG_SIG_BEACON_SYNC    6000    // Sync: Active 6ms +/- 10%
//#define LAZERTAG_SIG_BIT_ZERO       1000    // 0: Active 1ms +/- 10%
//#define LAZERTAG_SIG_BIT_ONE        2000    // 1: Active 2ms +/- 10%
//#define LAZERTAG_SIG_BIT_PAUSE      2000    // Pause: Inactive 2ms +/- 10%
//#define LAZERTAG_SIG_SFP            18000   // Special Format Pause: 18ms +/- 10%

//+=============================================================================
//
#if SEND_BRX
void  IRsend::sendBRX (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
    mark (HDR_MARK);
	space(HDR_SPACE);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			space(BIT_SPACE);
			mark (ONE_BIT);
		} else {
			space(BIT_SPACE);
			mark (ZERO_BIT);
		}
	}

	// Footer
	space(LONG_PAUSE);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_BRX
bool  IRrecv::decodeBRX(decode_results *results)
{
	unsigned long   data            = 0;    // Somewhere to build our code
	int             offset          = 1;    // Skip the Gap reading

//    int             tagLength       = 16;
//    int             beaconLength    = 12;
    
    //Clear
    results->bits = 0;
    
    //if (irparams.rawlen != 1 + 2 + (2 * BITS) + 1)  return false ;
    Serial.print("IRrecv::decodeBRX() - Raw Length = ");
    Serial.println(irparams.rawlen);
    Serial.println("-------------------------------------------------------------");
	
    // Check initial Mark+Space match
    if (!MATCH_MARK(results->rawbuf[offset++], HDR_MARK))
    {
        Serial.print("IRrecv::decodeBRX() - invalid HeaderMark = ");
        Serial.println(results->rawbuf[offset-1] * USECPERTICK, DEC);
        return false;
    }
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))
    {
        Serial.print("IRrecv::decodeBRX() - invalid HeaderSpace = ");
        Serial.println(results->rawbuf[offset-1] * USECPERTICK, DEC);
        return false ;
    }

	// Read the bits in
    
	for (int i = 0; i < BITS; i++)
	{
		// Each bit looks like: SPACE + MARK_1 -> 1
		//                 or : SPACE + MARK_0 -> 0

		if (!MATCH_SPACE(results->rawbuf[offset++], BIT_SPACE))
        {
            Serial.print("IRrecv::decodeBRX() - invalid BitSpace = ");
            Serial.print(results->rawbuf[offset-1] * USECPERTICK, DEC);
            Serial.print("\t in bit number ");
            Serial.println(offset-1);
            //return false;
        }
        
		// IR data is big-endian, so we shuffle it in from the right:
		if (MATCH_MARK(results->rawbuf[offset], ONE_BIT))
		{
			data = (data << 1) | 1;
			results->bits++;
            Serial.print("IRrecv::decodeBRX() - ONE");
            Serial.print("\t in bit number ");
            Serial.println(offset);
		}
		else if (MATCH_MARK(results->rawbuf[offset], ZERO_BIT))
		{
			data = (data << 1) | 0;
			results->bits++;
            Serial.print("IRrecv::decodeBRX() - ZERO");
            Serial.print("\t in bit number ");
             Serial.println(offset);
		}
        else if (MATCH_SPACE(results->rawbuf[offset], BIT_SPACE))
        {
            Serial.print("IRrecv::decodeBRX() - SPACE");
            Serial.print("\t in bit number ");
             Serial.println(offset);
        }
        else
        {
            Serial.print("IRrecv::decodeBRX() - invalid 1/0 bit = ");
            Serial.print(results->rawbuf[offset] * USECPERTICK, DEC);
            Serial.print("\t in bit number ");
            Serial.println(offset);
        }
		offset++;
	}

	// Success
	results->value = data;
	results->decode_type = BRX;
	
    Serial.print("\n\tSuccess = BRX : ");
    Serial.println (data);
    Serial.print("Number of bits = ");
    Serial.println(results->bits);
    Serial.println("-------------------------------------------------------------");

	return true;
}
#endif
