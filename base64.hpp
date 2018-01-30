#ifndef BASE64_HPP
#define BASE64_HPP

/*
 * functions for encoding sequences that are convertible to chars into
 * base64 encoded bytes into memory that is assignable from chars.
 * There is also a function for the reverse
 */

#include<type_traits> //for SFINAE

namespace base64 {

namespace detail {
    static const char * const encoding =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned char decode_elem(unsigned char elem)
    {
        if('0' <= elem && '9' >= elem)
        {
            return elem - '0' + 52;
        }
        if('A' <= elem && 'Z' >= elem)
        {
            return elem - 'A' + 0;
        }
        if('a' <= elem && 'z' >= elem)
        {
            return elem - 'a' + 26;
        }
        if('+' == elem)
        {
            return 62;
        }
        if('/' == elem)
        {
            return 63;
        }
        return 64;
    }

    unsigned char get_6bit(unsigned int input, int n)
    {
        return (input & (0x3f << n*6)) >> n*6;
    }

    unsigned int encode1byte(unsigned int input)
    {
        //the data is encoded as 0x00XX0000

        using byte = unsigned char;

        byte output_pack[4];
        output_pack[0] = '=';
        output_pack[1] = '=';
        output_pack[2] = get_6bit(input, 2);
        output_pack[3] = get_6bit(input, 3);

        //encode the bytes;
        output_pack[2] = encoding[output_pack[2]];
        output_pack[3] = encoding[output_pack[3]];

        return *reinterpret_cast<unsigned int*>(output_pack);
    }

    unsigned int encode2byte(unsigned int input)
    {
        //the data is encoded as 0x00XXXX00

        using byte = unsigned char;

        byte output_pack[4];
        output_pack[0] = '=';
        output_pack[1] = get_6bit(input, 1);
        output_pack[2] = get_6bit(input, 2);
        output_pack[3] = get_6bit(input, 3);

        //encode the bytes;
        output_pack[1] = encoding[output_pack[1]];
        output_pack[2] = encoding[output_pack[2]];
        output_pack[3] = encoding[output_pack[3]];

        return *reinterpret_cast<unsigned int*>(output_pack);
    }

    unsigned int encode3byte(unsigned int input)
    {
        //the data is encoded as 0x00XXXXXX

        using byte = unsigned char;

        byte output_pack[4];
        output_pack[0] = get_6bit(input, 0);
        output_pack[1] = get_6bit(input, 1);
        output_pack[2] = get_6bit(input, 2);
        output_pack[3] = get_6bit(input, 3);

        //encode the bytes;
        output_pack[0] = encoding[output_pack[0]];
        output_pack[1] = encoding[output_pack[1]];
        output_pack[2] = encoding[output_pack[2]];
        output_pack[3] = encoding[output_pack[3]];

        return *reinterpret_cast<unsigned int*>(output_pack);
    }

    unsigned char get_byte(unsigned int input, int n)
    {
        return (input & (0xff << n*8)) >> n*8;
    }

    unsigned int decode_pack(unsigned int pack)
    {
        return pack;
    }

}

/**
 * encode()
 *  will convert a sequence of (char) into another sequence of (char)
 *  that is base64 encoded. The output sequence should be at least 4/3
 *  the number of elements of the input sequence. It will return the
 *  iterator one past the last element outputed
 */
template<typename InIt, typename OutIt>
//SFINAE not implemented, be careful!
OutIt encode(InIt start, InIt end, OutIt out)
{
    using byte = unsigned char;
    while(start != end)
    {
        //I need a sequence of 3 bytes to convert to 4 base64 bytes
        byte input_pack[3];
        unsigned int inter_pack;
        input_pack[0] = *start;
        if(++start != end)
        {
            input_pack[1] = *start;
            if(++start != end)
            {
                input_pack[2] = *start;
                ++start;
                //encode 3 bytes
                inter_pack = (input_pack[0] << 16)
                           + (input_pack[1] << 8 )
                           + (input_pack[2] << 0 );
                inter_pack = detail::encode3byte(inter_pack);
            }
            else
            {
                //encode 2 bytes
                inter_pack = (input_pack[0] << 16)
                           + (input_pack[1] << 8 );
                inter_pack = detail::encode2byte(inter_pack);
            }
        }
        else
        {
            //encode 1 byte
            inter_pack = input_pack[0] << 16;
            inter_pack = detail::encode1byte(inter_pack);
        }
        byte output_pack[4];
        output_pack[0] = detail::get_byte(inter_pack, 3);
        output_pack[1] = detail::get_byte(inter_pack, 2);
        output_pack[2] = detail::get_byte(inter_pack, 1);
        output_pack[3] = detail::get_byte(inter_pack, 0);
        *out = output_pack[0];
        *(++out) = output_pack[1];
        *(++out) = output_pack[2];
        *(++out) = output_pack[3];
        ++out;
    }
    return out;
}

template<typename InIt, typename OutIt>
//SFINAE not implemented
OutIt decode(InIt start, InIt end, OutIt out)
{
    using byte = unsigned char;

    while(start != end)
    {
        byte input_pack[4] = {0};
        unsigned int inter_pack;
        input_pack[0] = *start;
        if(++start != end)
        {
            input_pack[1] = *start;
            if(++start != end)
            {
                input_pack[2] = *start;
                if(++start != end)
                {
                    input_pack[3] = *start;
                    ++start;
                }
            }
        }
        for(int i=0; i<4; i++)
        {
            input_pack[i] = ('=' == input_pack[i]) ? 'A' : input_pack[i];
        }

        for(int i=0; i<4; i++)
        {
            input_pack[i] = detail::decode_elem(input_pack[i]);
        }
        inter_pack = (input_pack[0] << 18)
                   + (input_pack[1] << 12)
                   + (input_pack[2] << 6)
                   + (input_pack[3] << 0);

        byte output_pack[3];
        output_pack[0] = detail::get_byte(inter_pack, 2);
        output_pack[1] = detail::get_byte(inter_pack, 1);
        output_pack[2] = detail::get_byte(inter_pack, 0);

        *out = output_pack[0];
        *(++out) = output_pack[1];
        *(++out) = output_pack[2];
        ++out;
    }
    return out;
}

}


#endif // BASE64_HPP
