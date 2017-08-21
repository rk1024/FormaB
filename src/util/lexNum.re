#include <cstdint>

namespace fun {
class LexNum {
  static int lex(const char *str) {
    const char *YYMARKER, *tSign, *tDigits, *tExpon, *tSpec /*!stags:re2c
          format = ", *@@";
        */;

    /*!re2c
      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = str;
      re2c:yyfill:enable = 0;

      nul = "\x00";

      hex_digit = [0-9a-fA-F_];
      dec_digit = [0-9_];
      oct_digit = [0-7_];
      bin_digit = [01_];
      sgn = [-+];
      dot = ".";
      expon = [eE] sgn? dec_digit+;
      int_type_spec = [uU]? [lLsS];
      float_type_spec = [fFdD];
      dec_type_spec = (int_type_spec | float_type_spec);

      hex = sgn? @tSign "0x" @tDigits hex_digit+ @tSpec int_type_spec?;

      oct = sgn? @tSign "0o" @tDigits oct_digit+ @tSpec int_type_spec?;

      bin = sgn? @tSign "0b" @tDigits bin_digit+ @tExpon expon? @tSpec
        int_type_spec?;

      dec = sgn? @tSign @tDigits dec_digit+ @tExpon expon? @tSpec
        dec_type_spec?;

      float = sgn? @tSign @tDigits dec_digit* dot dec_digit+ expon?
        @tSpec float_type_spec?;



      * { return -1; }
      hex nul { return 16; }
      oct nul { return 8; }
      bin nul { return 2; }
      dec nul { return 10; }
      float nul { return 0; }
    */
  }
};
}
