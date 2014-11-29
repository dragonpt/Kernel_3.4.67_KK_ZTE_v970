#ifndef _MT6577_M4U_REG_H
#define _MT6577_M4U_REG_H

#include <asm/io.h>
#include <mach/mt_reg_base.h>
	 
/* ===============================================================
 * 					  M4U definition
 * =============================================================== */
#define M4UMACRO0000          SMI_BASE
#define M4UMACRO0001                SMI_LARB0_BASE
#define M4UMACRO0002                SMI_LARB1_BASE
#define M4UMACRO0003                SMI_LARB2_BASE
#define M4UMACRO0004                SMI_LARB3_BASE

#define M4UMACRO0005     0x800
#define M4UMACRO0006          0x810
#define M4UMACRO0007 0x1

#define M4UMACRO0008          0x814

///> Bit[31:12] Virtual address
#define M4UMACRO0009          0xFFFFF000
///> Bit11 Lock bit
#define M4UMACRO0010 (0x1<<11)
#define M4UMACRO0011 (0x1<<10)

#define M4UMACRO0012         0x818
#define M4UMACRO0013      0x820
#define M4UMACRO0014 (0x1<<11)

#define M4UMACRO0015        0x824
#define M4UMACRO0016      0x828
#define M4UMACRO0017        0x82C
#define M4UMACRO0018      0x830
#define M4UMACRO0019        0x834
#define M4UMACRO0020      0x838
#define M4UMACRO0021        0x83C
#define M4UMACRO0022      0x840
#define M4UMACRO0023        0x844
#define M4UMACRO0024      0x848
#define M4UMACRO0025        0x84C
#define M4UMACRO0026      0x850
#define M4UMACRO0027        0x854
#define M4UMACRO0028      0x858
#define M4UMACRO0029        0x85C
#define M4UMACRO0030        0x880
#define M4UMACRO0031        0x884
#define M4UMACRO0032        0x888
#define M4UMACRO0033        0x88C
#define M4UMACRO0034        0x890
#define M4UMACRO0035        0x894
#define M4UMACRO0036       0x8C0
#define M4UMACRO0037       0x8C4
#define M4UMACRO0038         0x8F0
#define M4UMACRO0039         0x8F4
#define M4UMACRO0040        0x900
#define M4UMACRO0041        0x904
#define M4UMACRO0042        0x908
#define M4UMACRO0043        0x90C
#define M4UMACRO0044        0x910
#define M4UMACRO0045        0x914
#define M4UMACRO0046        0x918
#define M4UMACRO0047        0x91C
#define M4UMACRO0048        0x920
#define M4UMACRO0049        0x924
#define M4UMACRO00410       0x928
#define M4UMACRO00411       0x92C
#define M4UMACRO00412       0x930
#define M4UMACRO00413       0x934
#define M4UMACRO00414       0x938
#define M4UMACRO00415       0x93C
#define M4UMACRO00416       0x940
#define M4UMACRO00417       0x944
#define M4UMACRO00418       0x948
#define M4UMACRO00419       0x94C
#define M4UMACRO00420       0x950
#define M4UMACRO00421       0x954
#define M4UMACRO00422       0x958
#define M4UMACRO00423       0x95C
#define M4UMACRO00424       0x960
#define M4UMACRO00425       0x964
#define M4UMACRO00426       0x968
#define M4UMACRO00427       0x96C
#define M4UMACRO00428       0x970
#define M4UMACRO00429       0x974
#define M4UMACRO00430       0x978
#define M4UMACRO00431       0x97C
#define M4UMACRO0072         0x980
#define M4UMACRO0073         0x984
#define M4UMACRO0074         0x988
#define M4UMACRO0075         0x98C
#define M4UMACRO0076         0x990
#define M4UMACRO0077         0x994
#define M4UMACRO0078         0x998
#define M4UMACRO0079         0x99C
#define M4UMACRO0080         0x9A0
#define M4UMACRO0081         0x9A4
#define M4UMACRO00730        0x9A8
#define M4UMACRO00731        0x9AC
#define M4UMACRO00732        0x9B0
#define M4UMACRO00733        0x9B4
#define M4UMACRO00734        0x9B8
#define M4UMACRO00735        0x9BC
#define M4UMACRO00736        0x9C0
#define M4UMACRO00737        0x9C4
#define M4UMACRO00738        0x9C8
#define M4UMACRO00739        0x9CC
#define M4UMACRO00740        0x9D0
#define M4UMACRO00741        0x9D4
#define M4UMACRO00742        0x9D8
#define M4UMACRO00743        0x9DC
#define M4UMACRO00744        0x9E0
#define M4UMACRO00745        0x9E4
#define M4UMACRO00746        0x9E8
#define M4UMACRO00747        0x9EC
#define M4UMACRO00748        0x9F0
#define M4UMACRO00749        0x9F4
#define M4UMACRO00750        0x9F8
#define M4UMACRO00751        0x9FC
#define M4UMACRO0104       0xA00
#define M4UMACRO0105        0xA04
#define M4UMACRO0106         0xA10
#define M4UMACRO0107       0xA14
#define M4UMACRO0108      0xA20
#define M4UMACRO0109 (0x1<<12)

#define M4UMACRO0110         0xA24
#define M4UMACRO0111               0x01
#define M4UMACRO0112             0x02
#define M4UMACRO0113  0x04
#define M4UMACRO0114         0x08
#define M4UMACRO0115                0x10
#define M4UMACRO0116                  0x20
#define M4UMACRO0117     0x40
#define M4UMACRO0118          0x80

#define M4UMACRO0119         0xA28
#define M4UMACRO0120         0xA2C
#define M4UMACRO0121          0xA30
#define M4UMACRO0122       0xA34
#define M4UMACRO0123         0xA38
#define M4UMACRO0124           0xA3C
#define M4UMACRO0125         0xB00
#define M4UMACRO0126         0xB04
#define M4UMACRO0127         0xB08
#define M4UMACRO0128         0xB0C
#define M4UMACRO0129         0xB10
#define M4UMACRO0130         0xB14
#define M4UMACRO0131         0xB18
#define M4UMACRO0132         0xB1C
#define M4UMACRO0133         0xB40
#define M4UMACRO0134         0xB44
#define M4UMACRO0135         0xB48
#define M4UMACRO0136         0xC00
 /* ===============================================================
  * 					  SMI definition
  * =============================================================== */
#define M4UMACRO0137             0x0010
#define M4UMACRO0137_SET         0x0014
#define M4UMACRO0139  (1<<10)

#define M4UMACRO0137_RESET       0x0018
#define M4UMACRO0141              0x0020
#define M4UMACRO0141_SET          0x0024
#define M4UMACRO0141_CLR          0x0028
#define M4UMACRO0144          0x0030
#define M4UMACRO0145          0x0034
#define M4UMACRO0146            0x0038
#define M4UMACRO0147           0x0040
#define M4UMACRO0148           0x0044
#define M4UMACRO0149           0x0048
#define M4UMACRO0150              0x004C
#define M4UMACRO0151         0x0050
#define M4UMACRO0152         0x0054
#define M4UMACRO0153        0x0058
#define M4UMACRO0154        0x005C
#define M4UMACRO0155               0x0080
#define M4UMACRO0156               0x0084
#define M4UMACRO0157               0x0088
#define M4UMACRO0158             0x0100
#define M4UMACRO0159               0x0200
#define M4UMACRO0160               0x0204
#define M4UMACRO0161      0x0300
#define M4UMACRO0162        0x0304
#define M4UMACRO0163      0x0308
#define M4UMACRO0164      0x0310
#define M4UMACRO0165        0x0314
#define M4UMACRO0166      0x0318
#define M4UMACRO0167      0x0320
#define M4UMACRO0168        0x0324
#define M4UMACRO0169      0x0328
#define M4UMACRO0170 0x40000000
#define M4UMACRO0171 0x80000000
#define M4UMACRO0172 0x40000000

#define M4UMACRO0173         0x0400
#define M4UMACRO0174         0x0404
#define M4UMACRO0175        0x0408
#define M4UMACRO0176        0x040C
#define M4UMACRO0177         0x0410
#define M4UMACRO0178     0x0420
#define M4UMACRO0179     0x0424
#define M4UMACRO0180     0x0428
#define M4UMACRO0181     0x042C
#define M4UMACRO0182     0x0430
#define M4UMACRO0183      0x0434
#define M4UMACRO0184      0x0438
#define M4UMACRO0185     0x043C
#define M4UMACRO0186     0x0440
#define M4UMACRO0187    0x0450
#define M4UMACRO0188       0x0500
#define M4UMACRO0189       0x0504
#define M4UMACRO0190       0x0508
#define M4UMACRO0191 0x80
#define M4UMACRO0192 0x7F
#define M4UMACRO0193           0x0520

///> SMI_COMMON+
#define M4UMACRO0194       0x804
#define M4UMACRO0195 0x2
#define M4UMACRO0196 0x1
#define M4UMACRO0197         0x808
#define M4UMACRO0198         0x80C
#define M4UMACRO0199             0x820 //0x82C
#define M4UMACRO0200          0x9A0
#define M4UMACRO0201          0x9A4
#define M4UMACRO0202         0x9AC
#define M4UMACRO0203          0x9B0
#define M4UMACRO0204      0x9C0
#define M4UMACRO0205      0x9C4
#define M4UMACRO0206      0x9C8
#define M4UMACRO0207      0x9CC
#define M4UMACRO0208      0x9D0
#define M4UMACRO0209       0x9D4
#define M4UMACRO0210       0x9D8
#define M4UMACRO0211      0x9DC
#define M4UMACRO0212      0x9E0
#define M4UMACRO0213                0xA00
#define M4UMACRO0214               0xA04
#define M4UMACRO0215               0xA08
#define M4UMACRO0216               0xA0C
#define M4UMACRO0217               0xA10

static inline unsigned int M4U_ReadReg32(unsigned int M4uBase, unsigned int Offset) 
{
  return ioread32(M4uBase+Offset);
}
static inline void M4U_WriteReg32(unsigned int M4uBase, unsigned int Offset, unsigned int Val) 
{                   
  iowrite32(Val, M4uBase+Offset);  
  mb();
}

static inline unsigned int SMI_ReadReg32(unsigned int Offset) 
{
  return ioread32(M4UMACRO0000+Offset);
}
static inline void SMI_WriteReg32(unsigned int Offset, unsigned int Val)
{                   
  iowrite32(Val, M4UMACRO0000+Offset);
  mb();
}

#endif //_MMU_REG_H

