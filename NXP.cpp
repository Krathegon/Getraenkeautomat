/* 
 * File:   NXP.cpp
 * Author: fabian
 * 
 * Created on 27. August 2015, 22:26
 */

#include "NXP.h"

NXP nxp;

NXP::~NXP() {
    phhalHw_FieldOff(pHal);
}

bool NXP::init()
{
    /* Initialize the Reader BAL (Bus Abstraction Layer) component */
    status = phbalReg_RpiSpi_Init(&spi_balReader, sizeof(phbalReg_RpiSpi_DataParams_t));
    if (PH_ERR_SUCCESS != status)
    {
        printf("Failed to initialize SPI\n");
        return false;
    }
    balReader = (void *)&spi_balReader;

    status = phbalReg_OpenPort((void*)balReader);
    if (PH_ERR_SUCCESS != status)
    {
        printf("Failed to open bal\n");
        return false;
    }

    /* we have a board with PN512,
     * but on the software point of view,
     * it's compatible to the RC523 */
    status = phhalHw_Rc523_Init(&halReader,
                                sizeof(phhalHw_Rc523_DataParams_t),
                                balReader,
                                0,
                                bHalBufferReader,
                                sizeof(bHalBufferReader),
                                bHalBufferReader,
                                sizeof(bHalBufferReader));
    pHal = &halReader;

    if (PH_ERR_SUCCESS != status)
    {
        printf("Failed to initialize the HAL\n");
        return false;
    }

    /* Set the HAL configuration to SPI */
    status = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_BAL_CONNECTION,
                               PHHAL_HW_BAL_CONNECTION_SPI);
    if (PH_ERR_SUCCESS != status)
    {
        printf("Failed to set hal connection SPI\n");
        return false;
    }
    
    return true;
}

bool NXP::detectCard()
{
    if (DetectMifare())
    {
       /* reset the IC  */
       readerIC_Cmd_SoftReset();
       return true;
    }
    return false;
}

Card NXP::getCard()
{
    return card;
}

uint32_t NXP::DetectMifare()
{
    phpalI14443p4_Sw_DataParams_t I14443p4;
    phpalMifare_Sw_DataParams_t palMifare;
    phpalI14443p3a_Sw_DataParams_t I14443p3a;

    uint8_t cryptoEnc[8];
    uint8_t cryptoRng[8];

    phalMful_Sw_DataParams_t alMful;

    uint8_t bUid[10];
    uint8_t bLength;
    uint8_t bMoreCardsAvailable;
    uint32_t sak_atqa = 0;
    uint8_t pAtqa[2];
    uint8_t bSak[1];
    phStatus_t status;
    uint16_t detected_card = 0;


    /* Initialize the 14443-3A PAL (Protocol Abstraction Layer) component */
    PH_CHECK_SUCCESS_FCT(status, phpalI14443p3a_Sw_Init(&I14443p3a,
        sizeof(phpalI14443p3a_Sw_DataParams_t), (void*)&halReader));

    /* Initialize the 14443-4 PAL component */
    PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Sw_Init(&I14443p4,
        sizeof(phpalI14443p4_Sw_DataParams_t), (void*)&halReader));

    /* Initialize the Mifare PAL component */
    PH_CHECK_SUCCESS_FCT(status, phpalMifare_Sw_Init(&palMifare,
        sizeof(phpalMifare_Sw_DataParams_t), (void*)&halReader, &I14443p4));

    /* Initialize Ultralight(-C) AL component */
    PH_CHECK_SUCCESS_FCT(status, phalMful_Sw_Init(&alMful,
        sizeof(phalMful_Sw_DataParams_t), &palMifare, NULL, NULL, NULL));

    /* Reset the RF field */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset((void*)&halReader));

    /* Apply the type A protocol settings
     * and activate the RF field. */
    PH_CHECK_SUCCESS_FCT(status,
        phhalHw_ApplyProtocolSettings((void*)&halReader, PHHAL_HW_CARDTYPE_ISO14443A));

    /* Empty the pAtqa */
    memset(pAtqa, '\0', 2);
    status = phpalI14443p3a_RequestA(&I14443p3a, pAtqa);

    /* Reset the RF field */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset((void*)&halReader));

    /* Empty the bSak */
    memset(bSak, '\0', 1);

    /* Activate one card after another
     * and check it's type. */
    bMoreCardsAvailable = 1;
    //uint8_t cards = 0;
    while (bMoreCardsAvailable)
    {
        detected_card = 0;
        //cards++;
        /* Activate the communication layer part 3
         * of the ISO 14443A standard. */
        status = phpalI14443p3a_ActivateCard(&I14443p3a,
                        NULL, 0x00, bUid, &bLength, bSak, &bMoreCardsAvailable);
        uint8_t pUidOut[10];

        sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
        sak_atqa &= 0xFFFF0FFF;

        if (!status)
        {
            if((bLength==4)&&(bUid[0]==0x08))
            {
                printf("Card with random uid found, ignoring...\n");
                return false;
            }
            
            // Detect mini or classic
            detected_card = detectMiniOrClassic(sak_atqa);

            // detect other types
            if(!detected_card) {
                sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
                detected_card = detectOther(sak_atqa);
            }
        }
        else
            // No MIFARE card is in the field
            return false;

        // There is a MIFARE card in the field, but we cannot determine it
        if (!detected_card)
        {
            printf("undetermined MIFARE card detected\n");
            return false;
        }
        
        safeCard(bLength, bUid, sak_atqa);
        
        phpalI14443p3a_HaltA(&I14443p3a);
    }
    return detected_card;
}

void NXP::safeCard(uint8_t bLength, uint8_t *bUid, uint32_t sak_atqa)
{
    uint8_t i;
    char idbit[5];
    char cardstring[32];
    idbit[0]=0;
    cardstring[0]=0;

    card.id.clear();
    card.type.clear();
    
    for(i = 0; i < bLength; i++)
    {
        snprintf(idbit,sizeof idbit,"%02X",bUid[i]);
        card.id.append(idbit);
    }

    snprintf(cardstring,sizeof cardstring, "%04X", sak_atqa);
    card.type = cardstring;
}

uint16_t NXP::detectMiniOrClassic(uint32_t sak_atqa)
{
    uint16_t detected_card = 0;

    switch (sak_atqa)
    {
        case sak_mfc_1k << 24 | atqa_mfc:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mfc_1k_infineon << 24 | atqa_mfp_s:
            printf("MIFARE Classic by Infineon detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mfc_4k << 24 | atqa_mfc:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mfp_2k_sl1 << 24 | atqa_mfp_s:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mini << 24 | atqa_mini:
            printf("MIFARE Mini detected\n");
            detected_card = mifare_mini;
            break;
        case sak_mfp_4k_sl1 << 24 | atqa_mfp_s:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mfp_2k_sl1 << 24 | atqa_mfp_x:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
        case sak_mfp_4k_sl1 << 24 | atqa_mfp_x:
            printf("MIFARE Classic detected\n");
            detected_card = mifare_classic;
            break;
    }

    return detected_card;
}

uint16_t NXP::detectOther(uint32_t sak_atqa)
{
    uint16_t detected_card = 0;

    switch (sak_atqa)
    {
        case sak_ul << 24 | atqa_ul:
            printf("MIFARE Ultralight detected\n");
            detected_card = mifare_ultralight;
            break;
        case sak_mfp_2k_sl2 << 24 | atqa_mfp_s:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_2k_sl3 << 24 | atqa_mfp_s_2K:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_2k_sl3 << 24 | atqa_mfp_s:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_4k_sl2 << 24 | atqa_mfp_s:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_2k_sl2 << 24 | atqa_mfp_x:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_2k_sl3 << 24 | atqa_mfp_x:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_mfp_4k_sl2 << 24 | atqa_mfp_x:
            printf("MIFARE Plus detected\n");
            detected_card = mifare_plus;
            break;
        case sak_desfire << 24 | atqa_desfire:
            printf("MIFARE DESFire detected\n");
            detected_card = mifare_desfire;
            break;
        case sak_jcop << 24 | atqa_jcop:
            printf("JCOP detected\n");
            detected_card = jcop;
            break;
        case sak_layer4 << 24 | atqa_nPA:
        case sak_layer4 << 24 | atqa_nPA2:
            printf("German eID (neuer Personalausweis) detected\n");
            detected_card = nPA;
            break;
    }

    return detected_card;
}

phStatus_t NXP::readerIC_Cmd_SoftReset()
{
    phStatus_t status = PH_ERR_INVALID_DATA_PARAMS;

    switch (PH_GET_COMPID(&halReader))
    {
        case PHHAL_HW_RC523_ID:
            status = phhalHw_Rc523_Cmd_SoftReset(&halReader);
        break;
    }

    return status;
}
