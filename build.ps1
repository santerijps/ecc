$in = 'ecc.c'
$out = 'ecc.exe'

gcc -Ofast -s -fno-ident -fno-asynchronous-unwind-tables -Wall -Wextra -std=c2x -o $out $in -DVERSION='"1.0.0"'

if (!$?) {
  exit
}

upx --best --lzma --ultra-brute -f --compress-icons=3 --8-bit --no-reloc --no-align $out | out-null
