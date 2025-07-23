#ifndef LCD_H
#define LCD_H

/*
 * LCD number segments. This may be specific to WinWing, in which case
 * I'll have to add a way of specifying the type.
 *
 *    --40--
 *  04|    |20
 *    --02--
 *  01|    |10
 *    --08--
 */
enum class LCDDigit
{
    NUMBER_0 = 0x40 | 0x20 | 0x10 | 0x08 | 0x1 | 0x4,
    NUMBER_1 = 0x20 | 0x10,
    NUMBER_2 = 0x40 | 0x20 | 0x02 | 0x01 | 0x08,
    NUMBER_3 = 0x40 | 0x20 | 0x02 | 0x10 | 0x08,
    NUMBER_4 = 0x04 | 0x02 | 0x20 | 0x10,
    NUMBER_5 = 0x40 | 0x04 | 0x02 | 0x10 | 0x08,
    NUMBER_6 = 0x40 | 0x04 | 0x02 | 0x10 | 0x08 | 0x01,
    NUMBER_7 = 0x40 | 0x20 | 0x10,
    NUMBER_8 = 0x7f,
    NUMBER_9 = 0x40 | 0x04 | 0x02 | 0x20 | 0x10 | 0x08,
    A = 0x40 | 0x04 | 0x20 | 0x02 | 0x01 | 0x10,
    B = 0x04 | 0x02 | 0x01 | 0x10 | 0x08,
    C = 0x04 | 0x01 | 0x40 | 0x08,
    E = 0x04 | 0x01 | 0x40 | 0x02 | 0x08,
    F = 0x04 | 0x01 | 0x40 | 0x02,
    U = 0x04 | 0x01 | 0x08 | 0x10 | 0x20,
    Y = 0x04 | 0x02 | 0x20 | 0x10 | 0x08,
};

static uint8_t getLCDDigit(int number, int digit)
{
    int div = 1;
    for (int i = 0; i < digit; i++)
    {
        div *= 10;
    }
    number /= div;
    number %= 10;

    switch (number)
    {
        using enum LCDDigit;
        case 0: return static_cast<uint8_t>(NUMBER_0);
        case 1: return static_cast<uint8_t>(NUMBER_1);
        case 2: return static_cast<uint8_t>(NUMBER_2);
        case 3: return static_cast<uint8_t>(NUMBER_3);
        case 4: return static_cast<uint8_t>(NUMBER_4);
        case 5: return static_cast<uint8_t>(NUMBER_5);
        case 6: return static_cast<uint8_t>(NUMBER_6);
        case 7: return static_cast<uint8_t>(NUMBER_7);
        case 8: return static_cast<uint8_t>(NUMBER_8);
        case 9: return static_cast<uint8_t>(NUMBER_9);
        default: return 0;
    }
}

#endif //LCD_H
