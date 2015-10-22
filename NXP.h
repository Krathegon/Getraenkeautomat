/* 
 * File:   NXP.h
 * Author: fabian
 *
 * Created on 27. August 2015, 22:26
 */

#ifndef NXP_H
#define	NXP_H

#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#ifdef	__cplusplus
extern "C" {
#endif


#include <ph_NxpBuild.h>
#include <ph_Status.h>

#include <phpalI14443p3a.h>
#include <phpalI14443p4.h>
#include <phpalI14443p4a.h>
#include <phalMful.h>
#include <phalMfc.h>
#include <phKeyStore.h>

#include <phpalFelica.h>
#include <phpalI14443p3b.h>

#include <phbalReg_RpiSpi.h>
    


#ifdef	__cplusplus
}
#endif

#include "Card.h"


#define sak_ul                0x00
#define sak_ulc               0x00
#define sak_mini              0x09
#define sak_mfc_1k            0x08
#define sak_mfc_1k_infineon   0x88
#define sak_mfc_4k            0x18
#define sak_mfp_2k_sl1        0x08
#define sak_mfp_4k_sl1        0x18
#define sak_mfp_2k_sl2        0x10
#define sak_mfp_4k_sl2        0x11
#define sak_mfp_2k_sl3        0x20
#define sak_mfp_4k_sl3        0x20
#define sak_desfire           0x20
#define sak_jcop              0x28
#define sak_layer4            0x20

#define atqa_ul               0x4400
#define atqa_ulc              0x4400
#define atqa_mfc              0x0200
#define atqa_mfp_s            0x0400
#define atqa_mfp_s_2K         0x4400
#define atqa_mfp_x            0x4200
#define atqa_desfire          0x4403
#define atqa_jcop             0x0400
#define atqa_mini             0x0400
#define atqa_nPA              0x0800
#define atqa_nPA2             0x0407

#define mifare_ultralight     0x01
#define mifare_ultralight_c   0x02
#define mifare_classic        0x03
#define mifare_classic_1k     0x04
#define mifare_classic_4k     0x05
#define mifare_plus           0x06
#define mifare_plus_2k_sl1    0x07
#define mifare_plus_4k_sl1    0x08
#define mifare_plus_2k_sl2    0x09
#define mifare_plus_4k_sl2    0x0A
#define mifare_plus_2k_sl3    0x0B
#define mifare_plus_4k_sl3    0x0C
#define mifare_desfire        0x0D
#define jcop                  0x0F
#define mifare_mini           0x10
#define nPA                   0x11

class NXP {
public:
    ~NXP();
    bool init();
    bool detectCard();
    Card getCard();
private:
    phbalReg_RpiSpi_DataParams_t spi_balReader;
    void *balReader;

    phhalHw_Rc523_DataParams_t halReader;
    void *pHal;
    phStatus_t status;
    uint8_t blueboardType;
    uint8_t volatile card_or_tag_detected;

    uint8_t bHalBufferReader[0x40];
    
    Card card;

    // Forward declarations
    uint32_t DetectMifare();
    uint16_t detectMiniOrClassic(uint32_t sak_atqa);
    uint16_t detectOther(uint32_t sak_atqa);
    phStatus_t readerIC_Cmd_SoftReset();
    
    void safeCard(uint8_t bLength, uint8_t *bUid, uint32_t sak_atqa);
};

extern NXP nxp;

#endif	/* NXP_H */

