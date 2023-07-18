
#define	heart 	0x81,0x82
#define	smilly	0x83,0x84

CharGen		ORG     0x4FF           ; Start For Char table
  ADDWF	PCL,F	
  DT  0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00    ; " "
  DT  0x00 , 0x00 , 0x79 , 0x00 , 0x00 , 0x00    ; "!"
  DT  0x00 , 0x70 , 0x00 , 0x70 , 0x00 , 0x00    ; """
  DT  0x14 , 0x7F , 0x14 , 0x7F , 0x14 , 0x00    ; "#"
  DT  0x12 , 0x2A , 0x7F , 0x2A , 0x24 , 0x00    ; "$"
  DT  0x62 , 0x64 , 0x08 , 0x13 , 0x23 , 0x00    ; "%"
  DT  0x36 , 0x49 , 0x55 , 0x22 , 0x05 , 0x00    ; "&"
  DT  0x00 , 0x50 , 0x60 , 0x00 , 0x00 , 0x00    ; "'"
  DT  0x00 , 0x1C , 0x22 , 0x41 , 0x00 , 0x00    ; "("
  DT  0x00 , 0x41 , 0x22 , 0x1C , 0x00 , 0x00    ; ")"
  DT  0x14 , 0x08 , 0x3E , 0x08 , 0x14 , 0x00    ; "*"
  DT  0x08 , 0x08 , 0x3E , 0x08 , 0x08 , 0x00    ; "+"
  DT  0x00 , 0x00 , 0x05 , 0x06 , 0x00 , 0x00    ; ","
  DT  0x08 , 0x08 , 0x08 , 0x08 , 0x08 , 0x00    ; "-"
  DT  0x00 , 0x00 , 0x03 , 0x03 , 0x00 , 0x00    ; "."
  DT  0x02 , 0x04 , 0x08 , 0x10 , 0x20 , 0x00    ; "/"
  DT  0x3E , 0x41 , 0x41 , 0x41 , 0x3E , 0x00    ; "0"
  DT  0x00 , 0x21 , 0x7F , 0x01 , 0x00 , 0x00    ; "1"
  DT  0x21 , 0x43 , 0x45 , 0x49 , 0x31 , 0x00    ; "2"
  DT  0x42 , 0x41 , 0x51 , 0x69 , 0x46 , 0x00    ; "3"
  DT  0x0C , 0x14 , 0x24 , 0x7F , 0x04 , 0x00    ; "4"
  DT  0x72 , 0x51 , 0x51 , 0x51 , 0x4E , 0x00    ; "5"
  DT  0x1E , 0x29 , 0x49 , 0x49 , 0x06 , 0x00    ; "6"
  DT  0x40 , 0x47 , 0x48 , 0x50 , 0x60 , 0x00    ; "7"
  DT  0x36 , 0x49 , 0x49 , 0x49 , 0x36 , 0x00    ; "8"
  DT  0x30 , 0x49 , 0x49 , 0x4A , 0x3C , 0x00    ; "9"
  DT  0x00 , 0x36 , 0x36 , 0x00 , 0x00 , 0x00    ; ":"
  DT  0x00 , 0x35 , 0x36 , 0x00 , 0x00 , 0x00    ; ";"
  DT  0x08 , 0x14 , 0x22 , 0x41 , 0x00 , 0x00    ; "<"
  DT  0x14 , 0x14 , 0x14 , 0x14 , 0x14 , 0x00    ; "="
  DT  0x00 , 0x41 , 0x22 , 0x14 , 0x08 , 0x00    ; ">"
  DT  0x20 , 0x40 , 0x45 , 0x48 , 0x30 , 0x00    ; "?"
  DT  0x26 , 0x49 , 0x4F , 0x41 , 0x3E , 0x00    ; "@"
  DT  0x3F , 0x44 , 0x44 , 0x44 , 0x3F , 0x00    ; "�"
  DT  0x7F , 0x49 , 0x49 , 0x49 , 0x06 , 0x00    ; "�"
  DT  0x7F , 0x49 , 0x49 , 0x49 , 0x36 , 0x00    ; "�"
  DT  0x7F , 0x40 , 0x40 , 0x40 , 0x00 , 0x00    ; "�"
  DT  0x07 , 0x7C , 0x44 , 0x7C , 0x07 , 0x00    ; "�"
  DT  0x7F , 0x49 , 0x49 , 0x49 , 0x00 , 0x00    ; "�"
  DT  0x63 , 0x14 , 0x7F , 0x14 , 0x63 , 0x00    ; "�"
  DT  0x22 , 0x41 , 0x41 , 0x49 , 0x36 , 0x00    ; "�"
  DT  0x7F , 0x04 , 0x08 , 0x10 , 0x7F , 0x00    ; "�"
  DT  0x3F , 0x84 , 0x48 , 0x90 , 0x3F , 0x00    ; "�"
  DT  0x7F , 0x08 , 0x14 , 0x22 , 0x41 , 0x00    ; "�"
  DT  0x1F , 0x20 , 0x40 , 0x40 , 0x3F , 0x00    ; "�"
  DT  0x7F , 0x20 , 0x18 , 0x20 , 0x7F , 0x00    ; "�"
  DT  0x7F , 0x08 , 0x08 , 0x08 , 0x7F , 0x00    ; "�"
  DT  0x3E , 0x41 , 0x41 , 0x41 , 0x3E , 0x00    ; "�"
  DT  0x7F , 0x40 , 0x40 , 0x40 , 0x7F , 0x00    ; "�"
  DT  0x7F , 0x48 , 0x48 , 0x48 , 0x30 , 0x00    ; "�"
  DT  0x3E , 0x41 , 0x41 , 0x41 , 0x22 , 0x00    ; "�"
  DT  0x40 , 0x40 , 0x7F , 0x40 , 0x40 , 0x00    ; "�"
  DT  0x72 , 0x09 , 0x09 , 0x09 , 0x7E , 0x00    ; "�"
  DT  0x38 , 0x44 , 0x7F , 0x44 , 0x38 , 0x00    ; "�"
  DT  0x63 , 0x14 , 0x08 , 0x14 , 0x63 , 0x00    ; "�"
  DT  0x7E , 0x02 , 0x02 , 0x7E , 0x03 , 0x00    ; "�"
  DT  0x70 , 0x08 , 0x08 , 0x08 , 0x7F , 0x00    ; "�"
  DT  0x7F , 0x01 , 0x3F , 0x01 , 0x7F , 0x00    ; "�"
  DT  0x7F , 0x01 , 0x3F , 0x01 , 0x7F , 0x01    ; "�"
  DT  0x40 , 0x7F , 0x11 , 0x11 , 0x0E , 0x00    ; "�"
  DT  0x7F , 0x11 , 0x11 , 0x0E , 0x00 , 0x7F    ; "�"
  DT  0x00 , 0x7F , 0x11 , 0x11 , 0x0E , 0x00    ; "�"
  DT  0x22 , 0x41 , 0x49 , 0x49 , 0x3E , 0x00    ; "�"
  DT  0x7F , 0x08 , 0x3E , 0x41 , 0x41 , 0x3E    ; "�"
  DT  0x31 , 0x4A , 0x4C , 0x48 , 0x7F , 0x00    ; "�"
  DT  0x06 , 0x29 , 0x29 , 0x29 , 0x1F , 0x00    ; "�"
  DT  0x1E , 0x29 , 0x29 , 0x29 , 0x06 , 0x00    ; "�"
  DT  0x3F , 0x29 , 0x29 , 0x29 , 0x16 , 0x00    ; "�"
  DT  0x3F , 0x20 , 0x20 , 0x20 , 0x00 , 0x00    ; "�"
  DT  0x03 , 0x3C , 0x24 , 0x3C , 0x03 , 0x00    ; "�"
  DT  0x1E , 0x25 , 0x25 , 0x25 , 0x19 , 0x00    ; "�"
  DT  0x37 , 0x08 , 0x3F , 0x08 , 0x37 , 0x00    ; "�"
  DT  0x12 , 0x21 , 0x29 , 0x29 , 0x16 , 0x00    ; "�"
  DT  0x3F , 0x02 , 0x04 , 0x08 , 0x3F , 0x00    ; "�"
  DT  0x3F , 0x84 , 0x88 , 0x3F , 0x00 , 0x00    ; "�"
  DT  0x3F , 0x08 , 0x14 , 0x23 , 0x00 , 0x00    ; "�"
  DT  0x0F , 0x10 , 0x20 , 0x3F , 0x00 , 0x00    ; "�"
  DT  0x3F , 0x10 , 0x08 , 0x10 , 0x3F , 0x00    ; "�"
  DT  0x3F , 0x08 , 0x08 , 0x3F , 0x00 , 0x00    ; "�"
  DT  0x1E , 0x21 , 0x21 , 0x1E , 0x00 , 0x00    ; "�"
  DT  0x3F , 0x20 , 0x20 , 0x3F , 0x00 , 0x00    ; "�"
  DT  0x3F , 0x24 , 0x24 , 0x18 , 0x00 , 0x00    ; "�"
  DT  0x1E , 0x21 , 0x21 , 0x21 , 0x12 , 0x00    ; "�"
  DT  0x20 , 0x20 , 0x3F , 0x20 , 0x20 , 0x00    ; "�"
  DT  0x38 , 0x05 , 0x05 , 0x05 , 0x3E , 0x00    ; "�"
  DT  0x1C , 0x22 , 0x3F , 0x22 , 0x1C , 0x00    ; "�"
  DT  0x23 , 0x14 , 0x08 , 0x14 , 0x23 , 0x00    ; "�"
  DT  0x3E , 0x02 , 0x02 , 0x3E , 0x03 , 0x00    ; "�"
  DT  0x38 , 0x04 , 0x04 , 0x04 , 0x3F , 0x00    ; "�"
  DT  0x3F , 0x01 , 0x0F , 0x01 , 0x3F , 0x00    ; "�"
  DT  0x3F , 0x01 , 0x0F , 0x01 , 0x3F , 0x01    ; "�"
  DT  0x3F , 0x09 , 0x09 , 0x06 , 0x00 , 0x00    ; "�"
  DT  0x3F , 0x09 , 0x09 , 0x06 , 0x3F , 0x00    ; "�"
  DT  0x20 , 0x3F , 0x09 , 0x09 , 0x06 , 0x00    ; "�"
  DT  0x12 , 0x21 , 0x29 , 0x29 , 0x1E , 0x00    ; "�"
  DT  0x3F , 0x08 , 0x1E , 0x21 , 0x1E , 0x00    ; "�"
  DT  0x19 , 0x25 , 0x26 , 0x24 , 0x3F , 0x00    ; "�"
  DT  0x30 , 0x78 , 0x7C , 0x7C , 0x3E , 0x1F    ; 1 of 2 Heart
  DT  0x3E , 0x7C , 0x7C , 0x78 , 0x30 , 0x00    ; 2 of 2 Heart
  DT  0x00 , 0x1C , 0x3E , 0x77 , 0x5B , 0x7B    ; 1 of 2 Smilly
  DT  0x5B , 0x77 , 0x3E , 0x1C , 0x00 , 0x00    ; 2 of 2 Smilly
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x04 , 0x00 , 0x00 , 0x00    ; "�"
  DT  0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00    ; "�"

CharGenEnd
