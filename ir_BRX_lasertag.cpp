
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                          BBBB     RRRR      X     X
//                          B   B    R   R      X   X
//                          B   B    R   R       X X
//                          BBBB     RRRR         X
//                          B   B    R   R       X X
//                          B   B    R    R     X   X
//                          BBBB     R     R   X     X
//==============================================================================

//#define BITS                          25    // The number of bits in the command
//                                    // 4    = Bullet Type
//                                    // 6    = Player ID
//                                    // 2    = TeamID
//                                    // 8    = Damage
//                                    // 3    = Unknown
//                                    // 2    = Parity
//
//#define HDR_MARK                    2000    // The length of the Header:Mark
//#define HDR_SPACE                    500    // The length of the Header:Space
//
//#define ZERO_BIT                     500    // The length of a Bit:Mark for 0's
//#define ONE_BIT                     1000    // The length of a Bit:Mark for 1's
//#define BIT_SPACE                    500    // The length of a Bit:Space
//#define LONG_PAUSE                  5000	// The length between packets.


#define BITS          25  // The number of bits in the command
// 4    = Bullet Type
// 6    = Player ID
// 2    = TeamID
// 8    = Damage
// 3    = Unknown
// 2    = Parity

#define HDR_MARK    2000  // The length of the Header:Mark
#define HDR_SPACE    500  // The lenght of the Header:Space

#define BIT_MARK     470  // The length of a Bit:Mark
#define ONE_SPACE   1000  // The length of a Bit:Space for 1's
#define ZERO_SPACE   500  // The length of a Bit:Space for 0's






//+=============================================================================
//

#if SEND_BRX
void  IRsend::sendBRX (unsigned long data,  int nbits)
{
    // Set IR carrier frequency
    enableIROut(38);
    
    // Header
    mark (HDR_MARK);
    //space(HDR_SPACE);
    
    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            mark (BIT_MARK);
            space(ONE_SPACE);
        } else {
            mark (BIT_MARK);
            space(ZERO_SPACE);
        }
    }
    
    // Footer
    mark(BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_BRX
bool  IRrecv::decodeBRX (decode_results *results)
{
    Serial.println("");
    Serial.println("\t**** IRrecv::decodeBRX called ****");
    
    unsigned long  data   = 0;  // Somewhere to build our code
    int            offset = 1;  // Skip the Gap reading
    
    // Check we have the right amount of data
    Serial.print("decodeBRX: raw length of ");
    Serial.print(irparams.rawlen);
    Serial.print(" = calculated length of ");
    Serial.println(2 + (2 * BITS));
    Serial.println("");
    //if (irparams.rawlen != 2 + (2 * BITS))  return false ;
    
    // Check initial Mark+Space match
    if (!MATCH_MARK (results->rawbuf[offset], HDR_MARK ))
    {
        Serial.print("\t\tERROR: decodeBRX - HDR_MARK mismatch - # ");
        Serial.print(offset-1);
        Serial.print("\t rawData = ");
        Serial.println(results->rawbuf[offset] * USECPERTICK);
        return false ;
    }
    else if(MATCH_MARK (results->rawbuf[offset], HDR_MARK ))
    {
        Serial.print("decodeBRX - HDR_MARK  - # ");
        Serial.print(offset);
        Serial.print("\t rawData = ");
        Serial.println(results->rawbuf[offset] * USECPERTICK);
    }
    offset++;
    
//    Serial.println("decodeBRX \t- Header OK, waiting for data");
//    Serial.println("");
    
    // Read the bits in
    for (int i = 0;  i < BITS;  i++) {
        // Each bit looks like: MARK + SPACE_1 -> 1
        //                 or : MARK + SPACE_0 -> 0

        if (!MATCH_MARK(results->rawbuf[offset], BIT_MARK))
        {
            Serial.print("\t\tERROR: decodeBRX - BIT_MARK mismatch - # ");
            Serial.print(offset);
            Serial.print("\t rawData = ");
            Serial.println(results->rawbuf[offset] * USECPERTICK);
            // return false ;
        }
        else if (MATCH_MARK(results->rawbuf[offset], BIT_MARK))
        {
            Serial.print("decodeBRX -  BIT_MARK - # ");
            Serial.print(offset);
            Serial.print("\t rawData = ");
            Serial.println(results->rawbuf[offset] * USECPERTICK);
        }
        offset++;
        
        // IR data is big-endian, so we shuffle it in from the right:
        if      (MATCH_SPACE(results->rawbuf[offset], ONE_SPACE))
        {
            Serial.print("decodeBRX - ONE_SPACE - # ");
            Serial.print(offset);
            Serial.print("\t rawData = ");
            Serial.println(results->rawbuf[offset] * USECPERTICK);
            data = (data << 1) | 1 ;
        }
        else if (MATCH_SPACE(results->rawbuf[offset], ZERO_SPACE))
        {
            Serial.print("decodeBRX - ZERO_SPACE- # ");
            Serial.print(offset);
            Serial.print("\t rawData = ");
            Serial.println(results->rawbuf[offset] * USECPERTICK);
            data = (data << 1) | 0 ;
        }
        else
        {
            Serial.print("\t\tERROR: decodeBRX - ONE/ZERO_SPACE mismatch - # ");
            Serial.print(offset);
            Serial.print("\t rawData = ");
            Serial.println(results->rawbuf[offset] * USECPERTICK);
            // return false ;
        }
        offset++;
        
        
        
        
    }
    Serial.println("----------------------------------------------");
    Serial.println("        ***** BRX PACKET SUCCESS *****");
    
    // Success
    results->bits        = BITS;
    results->value       = data;
    results->decode_type = BRX;
    return true;
}
#endif



//void IRrecv::showDebug(String text, int bitIndex)
//{
//    Serial.print("decodeBRX - "); Serial.print(text); Serial.print(" - Bit # "); Serial.print(bitIndex);
//    Serial.print("\t rawData = ");Serial.println(results->rawbuf[bitIndex] * USECPERTICK);
//}









//#if SEND_BRX
//void  IRsend::sendBRX (unsigned long data,  int nbits)
//{
//    // Set IR carrier frequency
//    enableIROut(38);
//
//    // Header
//    mark (HDR_MARK);
//    space(HDR_SPACE);
//
//    // Data
//    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
//        if (data & mask) {
//            space(BIT_SPACE);
//            mark (ONE_BIT);
//        } else {
//            space(BIT_SPACE);
//            mark (ZERO_BIT);
//        }
//    }
//
//    // Footer
//    space(LONG_PAUSE);
//    space(0);  // Always end with the LED off
//}
//#endif
//
////+=============================================================================
////
//#if DECODE_BRX
//bool  IRrecv::decodeBRX(decode_results *results)
//{
//    Serial.print("***** IRrecv::decodeBRX() - called ***** ");
//
//    unsigned long   data            = 0;    // Somewhere to build our code
//    int             offset          = 1;    // Skip the Gap reading
//
////    int             tagLength       = 16;
////    int             beaconLength    = 12;
//
//    //Clear
//    results->bits = 0;
//
//    //if (irparams.rawlen != 1 + 2 + (2 * BITS) + 1)  return false ;
//    Serial.print("IRrecv::decodeBRX() - Raw Length = ");
//    Serial.println(irparams.rawlen);
//    Serial.println("-------------------------------------------------------------");
//
//    // Check initial Mark+Space match
//    if (!MATCH_MARK(results->rawbuf[offset++], HDR_MARK))
//    {
//        Serial.print("IRrecv::decodeBRX() - invalid HeaderMark = ");
//        Serial.println(results->rawbuf[offset-1] * USECPERTICK, DEC);
//        return false;
//    }
//    if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))
//    {
//        Serial.print("IRrecv::decodeBRX() - invalid HeaderSpace = ");
//        Serial.println(results->rawbuf[offset-1] * USECPERTICK, DEC);
//        return false ;
//    }
//
//    // Read the bits in
//
//    for (int i = 0; i < BITS; i++)
//    {
//        // Each bit looks like: SPACE + MARK_1 -> 1
//        //                 or : SPACE + MARK_0 -> 0
//
//        if (!MATCH_SPACE(results->rawbuf[offset++], BIT_SPACE))
//        {
//            Serial.print("IRrecv::decodeBRX() - invalid BitSpace = ");
//            Serial.print(results->rawbuf[offset-1] * USECPERTICK, DEC);
//            Serial.print("\t in bit number ");
//            Serial.println(offset-1);
//            //return false;
//        }
//
//        // IR data is big-endian, so we shuffle it in from the right:
//        if (MATCH_MARK(results->rawbuf[offset], ONE_BIT))
//        {
//            data = (data << 1) | 1;
//            results->bits++;
//            Serial.print("\tIRrecv::decodeBRX() - ONE");
//            Serial.print("\t in bit number ");
//            Serial.println(offset);
//        }
//        else if (MATCH_MARK(results->rawbuf[offset], ZERO_BIT))
//        {
//            data = (data << 1) | 0;
//            results->bits++;
//            Serial.print("\tIRrecv::decodeBRX() - ZERO");
//            Serial.print("\t in bit number ");
//            Serial.println(offset);
//        }
//        else if (MATCH_SPACE(results->rawbuf[offset], BIT_SPACE))
//        {
//            Serial.print("\tIRrecv::decodeBRX() - SPACE");
//            Serial.print("\t in bit number ");
//            Serial.println(offset);
//        }
//        else
//        {
//            Serial.print("IRrecv::decodeBRX() - invalid 1/0 bit = ");
//            Serial.print(results->rawbuf[offset] * USECPERTICK, DEC);
//            Serial.print("\t in bit number ");
//            Serial.println(offset);
//        }
//        offset++;
//    }
//    Serial.println("-------------------------------------------------------------");
//
//    // Success
//    results->value = data;
//    results->decode_type = BRX;
//
//    Serial.print("\n\t\tSuccess = BRX : ");
//    Serial.println (data);
//    Serial.print("Number of bits = ");
//    Serial.println(results->bits);
//    Serial.println("-------------------------------------------------------------");
//
//    return true;
//}
//#endif
