
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                          L      TTTTT  TTTTT   OOO
//                          L        T		T	 O   O
//                          L		 T		T	 O   O
//                          L		 T		T	 O   O
//                          LLLLL    T		T	  OOO
//==============================================================================

#define BITS                        32      // The number of bits in the command

#define HDR_MARK                    3000	// The length of the Header:Mark
#define HDR_SPACE                   6000	// The length of the Header:Space
#define TAG_SYNC                    3000	// The length of the Sync signal for a Tag:Mark
#define BEACON_SYNC                 6000	// The length of the Sync signal for a Beacon:Mark

#define ZERO_BIT                    1000	// The length of a Bit:Mark for 0's
#define ONE_BIT                     2000	// The length of a Bit:Mark for 1's
#define BIT_SPACE                   2000	// The length of a Bit:Space
#define LONG_PAUSE                  25000	// The length between packets.

#define TYPE_LAZERTAG_TAG           100		// Decode this as a tag
#define TYPE_LAZERTAG_BEACON        101		// Decode this a a beacon

#define LAZERTAG_SIG_PS             3000    // Presync: Active 3ms +/- 10%
#define LAZERTAG_SIG_PSP            6000    // Presync Pause: Inactive 6ms +/- 10%
#define LAZERTAG_SIG_TAG_SYNC       3000    // Sync: Active 3ms +/- 10%
#define LAZERTAG_SIG_BEACON_SYNC    6000	// Sync: Active 6ms +/- 10%
#define LAZERTAG_SIG_BIT_ZERO       1000	// 0: Active 1ms +/- 10%
#define LAZERTAG_SIG_BIT_ONE        2000	// 1: Active 2ms +/- 10%
#define LAZERTAG_SIG_BIT_PAUSE      2000    // Pause: Inactive 2ms +/- 10%
#define LAZERTAG_SIG_SFP            18000   // Special Format Pause: 18ms +/- 10%

//+=============================================================================
//
#if SEND_LTTO
void  IRsend::sendLTTO (unsigned long data,  int nbits, bool beacon)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark (HDR_MARK);		//PreSync
	space(HDR_SPACE);		//PreSync Pause

	//Sync
	if (beacon)	mark(BEACON_SYNC);
	else		mark(TAG_SYNC);

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
#if DECODE_LTTO
bool  IRrecv::decodeLTTO(decode_results *results)
{
	unsigned long   data            = 0;    // Somewhere to build our code
	int             offset          = 0;    // The IF(_GAP) statement sets the Skip the Gap reading
	int             headerSize      = 2;	// Number of bits for the header, altered by IF(_GAP) to 4
	int             tagLength       = 16;
	int             beaconLength    = 12;
    
    //Clear
    results->bits = 0;
	
    // Check initial Mark+Space match
	if (_GAP > 7000)
	{
		// The library default for _GAP is 5000, which means the 6000uS Space is seen as a new packet,
		// therefore the initial 3000uS Mark gets lost, so ignore it unless _GAP > 7000.
		headerSize	 = 4;		// allow for the 2 extra bits in the packet.
		tagLength	 = 18;		// allow for the 2 extra bits in the packet.
		beaconLength = 14;		// allow for the 2 extra bits in the packet.
		offset++;				// Skip the Gap reading
		if (!MATCH_MARK(results->rawbuf[offset++], HDR_MARK))
        {
            Serial.print("IRrecv::decodeLTTO() - invalid HeaderMark");
            Serial.println(results->rawbuf[offset+1]);
                           offset++;
            return false;
        }
	}
    
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))
    {
        Serial.print("IRrecv::decodeLTTO() - invalid HeaderSpace");
        Serial.println(results->rawbuf[offset-1]);
        return false ;
    }

    
	// Check the Sync Type
	if (MATCH_MARK(results->rawbuf[offset], TAG_SYNC))
	{
		if (results->rawlen < tagLength)
        {
            Serial.print("IRrecv::decodeLTTO() - invalid TagSync3000");
            Serial.println(results->rawbuf[offset]);
            return false;
        }
		results->address = 3000;			//TYPE_LAZERTAG_TAG;
	}
	else if (MATCH_MARK(results->rawbuf[offset], BEACON_SYNC))
	{
		if (results->rawlen < beaconLength)
        {
            Serial.print("IRrecv::decodeLTTO() - invalid BeaconSync6000");
            Serial.println(results->rawbuf[offset]);
            return false;
        }
		results->address = 6000;			//TYPE_LAZERTAG_BEACON;
	}
	else return false;
	offset++;

	// Read the bits in
	for (int i = 0; i < ((results->rawlen-headerSize)/2); i++)
	{
		// Each bit looks like: SPACE + MARK_1 -> 1
		//                 or : SPACE + MARK_0 -> 0

		if (!MATCH_SPACE(results->rawbuf[offset++], BIT_SPACE))
        {
            Serial.print("IRrecv::decodeLTTO() - invalid BitSpace");
            Serial.println(results->rawbuf[offset-1]);
            return false;
        }
        
		// IR data is big-endian, so we shuffle it in from the right:
		if (MATCH_MARK(results->rawbuf[offset], ONE_BIT))
		{
			data = (data << 1) | 1;
			results->bits++;
		}
		else if (MATCH_MARK(results->rawbuf[offset], ZERO_BIT))
		{
			data = (data << 1) | 0;
			results->bits++;
		}
		offset++;
	}

	// Success
	results->value = data;
	results->decode_type = LTTO;
	
    Serial.print("\n\tSuccess = LTTO : ");
    Serial.println (data);

	return true;
}
#endif
