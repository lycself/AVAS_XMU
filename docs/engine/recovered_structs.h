/*
 * recovered_structs.h  ——  libAVAS.so（GCC 4.8.5, x86-64, LP64）核心数据结构的恢复布局
 *
 * 说明见同目录 structures.md（每个字段的证据与把握程度都写在那里）。
 * 本文件只做两件事：给出能被 C 解析器接受的结构定义；在注释里标出偏移和"确定 / 推测"。
 *
 * 约定
 *   - 所有偏移都是十六进制字节偏移，sizeof 在每个结构末尾注明。
 *   - C++ 容器展开成 POD：std::vector<T> = {T* begin; T* end; T* cap;}（24 字节），
 *     std::string（GCC 4.8 COW 实现）= 一个 char*（8 字节，指向字符数据，引用计数头在其前 0x18 字节）。
 *   - 名字以 unk_ 开头的字段是只知道类型 / 位置、不知道含义的成员；pad_ 是对齐填充。
 *   - 单位：未注明处一律 SI（m、s、T、V/m、kg、C）；"γβ" 是无量纲动量 p/(mc)。
 *   - 元件类没有虚析构函数（vtable 只有 7 项，见 ElementVtbl）。
 *
 * 导入 Ghidra：File -> Parse C Source，加入本文件；若解析器找不到 <stdint.h>，
 * 在 Parse Options 里加 -DAVAS_GHIDRA_IMPORT 使用下面的内置 typedef。
 */
#ifndef AVAS_RECOVERED_STRUCTS_H
#define AVAS_RECOVERED_STRUCTS_H

#if defined(AVAS_GHIDRA_IMPORT)
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef long               int64_t;
typedef unsigned long      uint64_t;
#else
#include <stdint.h>
#endif

/* ------------------------------------------------------------------ 前置声明 */
typedef struct Particle       Particle;
typedef struct SynParticle    SynParticle;
typedef struct Beam           Beam;
typedef struct Element        Element;
typedef struct ElementVtbl    ElementVtbl;
typedef struct FieldMatrix    FieldMatrix;
typedef struct Lattice        Lattice;
typedef struct Geometry       Geometry;
typedef struct Boundary       Boundary;
typedef struct ErrorSample    ErrorSample;
typedef struct ErrorParameter ErrorParameter;
typedef struct Mesh           Mesh;
typedef struct PIC            PIC;
typedef struct Initial        Initial;
typedef struct MethodMP       MethodMP;

/* ------------------------------------------------------------------ C++ 容器的 POD 展开 */
typedef struct { char *p; } cxx_string;                         /* std::string, 8 字节 */

typedef struct { double   *begin, *end, *cap; } vec_double;      /* std::vector<double>  */
typedef struct { int32_t  *begin, *end, *cap; } vec_int;         /* std::vector<int>     */
typedef struct { cxx_string *begin, *end, *cap; } vec_string;    /* std::vector<std::string> */
typedef struct { vec_double *begin, *end, *cap; } vec_vec_double;           /* vector<vector<double>> */
typedef struct { vec_int    *begin, *end, *cap; } vec_vec_int;              /* vector<vector<int>>    */
typedef struct { vec_vec_double *begin, *end, *cap; } vec_vec_vec_double;   /* vector<vector<vector<double>>> */
typedef struct { Element  **begin, **end, **cap; } vec_element_ptr;         /* vector<Element*>  */
typedef struct { Geometry **begin, **end, **cap; } vec_geometry_ptr;        /* vector<Geometry*> */

/* ================================================================== Particle
 * 宏粒子。Beam::particles 用 new[] 分配 N+1 个（带 8 字节计数 cookie），下标 N 是同步（参考）粒子。
 * sizeof = 0x190 (400)，由 LeapFrog::Update(double, Beam&) 等处的 400 字节步长确定。
 * 构造函数（内联在 InitParticle 里）只写 +0xfc = 0 和 +0x188 = 空串。
 */
struct Particle {
    double  x;              /* 0x000 m                                            确定 */
    double  gbx;            /* 0x008 γβx                                          确定 */
    double  y;              /* 0x010 m                                            确定 */
    double  gby;            /* 0x018 γβy                                          确定 */
    double  z;              /* 0x020 m（实验室纵向坐标）                          确定 */
    double  gbz;            /* 0x028 γβz                                          确定 */
    double  extE[3];        /* 0x030 外场 Ex,Ey,Ez（元件 GetField 写入）V/m       确定 */
    double  extB[3];        /* 0x048 外场 Bx,By,Bz T                              确定 */
    double  scE[3];         /* 0x060 空间电荷场 E V/m                             确定 */
    double  scB[3];         /* 0x078 空间电荷场 B T（实现里恒为 0）               确定 */
    double  totE[3];        /* 0x090 推进用总电场 = ext + sc                       确定 */
    double  totB[3];        /* 0x0a8 推进用总磁场                                 确定 */
    double  rel[6];         /* 0x0c0 相对同步粒子的坐标 x-xs, γβx, y-ys, γβy, z-zs, γβz
                                     （弯转时按 Beam.rotAngle 旋转；空间电荷网格用） 确定 */
    int32_t elemIndex;      /* 0x0f0 当前所在元件下标                             确定 */
    int32_t geomIndex;      /* 0x0f4 当前所在 boundary 几何段下标                 确定 */
    int32_t unk_0f8;        /* 0x0f8 未见访问                                     推测 */
    int32_t spOutCount;     /* 0x0fc 已穿过的 superposeout 块数（t-code 主束）    推测 */
    int32_t lost;           /* 0x100 0 存活 / 1 丢失 / 2 越过 lattice 末端        确定 */
    int32_t nextPlane;      /* 0x104 下一个输出面序号                             推测 */
    double  spOutZ0;        /* 0x108 最近穿过的 superposeout 出口面 z（m）        推测 */
    double  spOutZ1;        /* 0x110 下一个 superposeout 出口面 z（m）            推测 */
    double  charge;         /* 0x118 宏粒子电荷 = N_real·w·Q·e（C）；同步粒子为 Q·e 确定 */
    double  mass;           /* 0x120 宏粒子质量 = N_real·w·A·m_p（kg）            确定 */
    double  qOverM;         /* 0x128 荷质比 q/m（C/kg）                           确定 */
    double  restMassMeV;    /* 0x130 静止能量 mc²（MeV）                          确定 */
    double  t;              /* 0x138 粒子自身时间（s），二次粒子用                确定 */
    double  dt;             /* 0x140 粒子自身步长（s），二次粒子用                确定 */
    double  unk_148[5];     /* 0x148 未见访问                                     推测 */
    int32_t id;             /* 0x170 粒子编号（二次粒子输出第一列）               确定 */
    int32_t spOutIndex;     /* 0x174 下一个 superposeout 变换序号（二次粒子）     推测 */
    int32_t boundaryMode;   /* 0x178 =input.txt boundary；0 时用元件孔径判丢失    确定 */
    int32_t pad_17c;        /* 0x17c                                              */
    double  weight;         /* 0x180 权重（默认 1，indweight 时读文件）           确定 */
    cxx_string kind;        /* 0x188 粒子种类名（kindofparticle）                 确定 */
};                          /* sizeof = 0x190 */

/* ================================================================== SynParticle
 * 同步粒子求相位用的独立对象（Initial::SetSynPhase / singlePartM 的局部变量）。
 * sizeof = 0xb8：SetSynPhase 栈上 local_918 到下一个对象（ofstream @ +0xc0）之间，最后写入的是 +0xb0。
 */
struct SynParticle {
    vec_double coord;       /* 0x00 [x, γβx, y, γβy, z, γβz]（6 个）              确定 */
    vec_double E;           /* 0x18 当前外电场（3 个）V/m                         确定 */
    vec_double B;           /* 0x30 当前外磁场（3 个）T                           确定 */
    vec_double cavPhase;    /* 0x48 每个射频腔的相位（deg）                       确定 */
    vec_double cavTime;     /* 0x60 每个射频腔的参考时间（s）                     确定 */
    int32_t elemIndex;      /* 0x78 当前元件下标                                  确定 */
    int32_t lost;           /* 0x7c 超出孔径时置 1                                确定 */
    int32_t fullPush;       /* 0x80 0 = 只做纵向推进（横向坐标清零）              确定 */
    int32_t pad_84;         /* 0x84                                               */
    double  charge;         /* 0x88 Q·e（C）                                      确定 */
    double  mass;           /* 0x90 A·m_p（kg）                                   确定 */
    double  qOverM;         /* 0x98 C/kg                                          确定 */
    double  restMassMeV;    /* 0xa0 MeV                                           确定 */
    double  EzAmp;          /* 0xa8 当前位置轴向电场幅值（未乘 cos 相位因子）     推测 */
    double  frequency;      /* 0xb0 束流频率 Hz                                   确定 */
};                          /* sizeof = 0xb8（推测，可能有 8 字节尾部对齐到 0xc0）*/

/* ================================================================== Beam
 * sizeof：main_agent 栈上 Beam(@-0x858) 与 Initial(@-0x768) 相距 0xf0；最后一个成员 +0xe0。
 */
struct Beam {
    vec_double centroid;    /* 0x00 加权平均 [x, γβx, y, γβy, z, γβz]（6）        确定 */
    vec_double rms;         /* 0x18 rms x, rms y, rms z·γ（3，m）                 确定 */
    vec_double syncCoord;   /* 0x30 同步粒子坐标副本（6）                         确定 */
    Particle  *particles;   /* 0x48 new[] 的 N+1 个宏粒子，[N] 为同步粒子         确定 */
    double  frequency;      /* 0x50 束流频率 Hz                                   确定 */
    int32_t n;              /* 0x58 宏粒子数 N（不含同步粒子）                    确定 */
    int32_t pad_5c;         /* 0x5c                                               */
    double  current;        /* 0x60 流强 A                                        确定 */
    int32_t nAlive;         /* 0x68 存活宏粒子数（DataSet 第 28 列）              确定 */
    int32_t scSolverRecord; /* 0x6c =input.txt scsolverrecord                     确定 */
    double  betaGammaLambda;/* 0x70 βγ·c/f（m）                                   确定 */
    double  gamma;          /* 0x78 平均 γ                                        确定 */
    double  bunchTime;      /* 0x80 束团时间长度（s），默认 3e-9                  确定 */
    double  tMin;           /* 0x88 初始束团最早时间（s）                         确定 */
    double  zMin;           /* 0x90 存活粒子最小 z（初值 +1e10）                  确定 */
    double  zMax;           /* 0x98 存活粒子最大 z（初值 -1e10）                  确定 */
    double  rotAngle;       /* 0xa0 弯转段参考坐标旋转角（rad）                   确定 */
    int32_t rotMode;        /* 0xa8 0 不旋转 / 1 竖直 / 2 水平                    推测 */
    int32_t unk_ac;         /* 0xac                                               推测 */
    double  seStep;         /* 0xb0 二次粒子步长参数 steppercycle V2（m）         确定 */
    int32_t unk_b8;         /* 0xb8                                               推测 */
    int32_t useWeightFile;  /* 0xbc indweight 读权重文件时为 1                    确定 */
    int32_t fromDstFile;    /* 0xc0 从 .dst 读初始分布时为 1                      确定 */
    int32_t pad_c4;         /* 0xc4                                               */
    double  totalWeight;    /* 0xc8 Σweight（无权重文件时 = N）                   确定 */
    double  realPerMacro;   /* 0xd0 每个宏粒子代表的真实粒子数                    确定 */
    int32_t recScField;     /* 0xd8 =beam.txt recscfield                          确定 */
    int32_t impScField;     /* 0xdc =beam.txt impscfield                          确定 */
    double  time;           /* 0xe0 当前模拟时间（s）                             确定 */
};                          /* sizeof = 0xe8（推测；栈间距允许 0xf0）*/

/* ================================================================== FieldMatrix
 * 场图库，嵌在 Lattice+0x18。sizeof = 0x1c0（Lattice 的下一个成员在 0x1d8）。
 */
struct FieldMatrix {
    vec_vec_vec_double maps;/* 0x000 [图][节点] -> {Ex,Ey,Ez,Bx,By,Bz}            确定 */
    vec_string names;       /* 0x018 场图文件名（查重用）                         确定 */
    vec_double cavPhase;    /* 0x030 每个射频腔的 RF 相位（deg）                  确定 */
    vec_double cavTime;     /* 0x048 每个射频腔的参考时间（s）                    确定 */
    int32_t nMaps;          /* 0x060 已载入场图数                                 确定 */
    int32_t pad_064;        /* 0x064                                              */
    vec_int nx;             /* 0x068 每张图 x 网格数                              确定 */
    vec_int ny;             /* 0x080                                              确定 */
    vec_int nz;             /* 0x098                                              确定 */
    vec_double xmin;        /* 0x0b0 m                                            确定 */
    vec_double xmax;        /* 0x0c8                                              确定 */
    vec_double ymin;        /* 0x0e0                                              确定 */
    vec_double ymax;        /* 0x0f8                                              确定 */
    vec_double zlen;        /* 0x110 场图纵向长度 m                               确定 */
    vec_double dx;          /* 0x128 网格步长                                     确定 */
    vec_double dy;          /* 0x140                                              确定 */
    vec_double dz;          /* 0x158                                              确定 */
    int32_t curNx;          /* 0x170 读文件时的临时量（随后 push 进上面的数组）   确定 */
    int32_t curNy;          /* 0x174                                              确定 */
    int32_t curNz;          /* 0x178                                              确定 */
    int32_t pad_17c;        /* 0x17c                                              */
    double  curXmin;        /* 0x180                                              确定 */
    double  curXmax;        /* 0x188                                              确定 */
    double  curYmin;        /* 0x190                                              确定 */
    double  curYmax;        /* 0x198                                              确定 */
    double  curZlen;        /* 0x1a0                                              确定 */
    double  curDx;          /* 0x1a8                                              确定 */
    double  curDy;          /* 0x1b0                                              确定 */
    double  curDz;          /* 0x1b8                                              确定 */
};                          /* sizeof = 0x1c0 */

/* ================================================================== Element 及其 vtable
 * vtable（_ZTV7Element，0x48 字节 = offset-to-top + typeinfo + 7 个槽）。没有虚析构。
 */
struct ElementVtbl {
    void (*GetFieldSyn)(Element *self, double t, SynParticle *p, const FieldMatrix *fm); /* +0x00 */
    void (*GetErrField)(Element *self, double t, Particle *p, const FieldMatrix *fm);    /* +0x08 */
    void (*GetField)(Element *self, double t, Particle *p, const FieldMatrix *fm);       /* +0x10 */
    void (*GetParameter)(Element *self, vec_double *out);                                 /* +0x18 */
    void (*SetErrPara)(Element *self, const ErrorSample *err);                            /* +0x20 */
    void (*SetParameter)(Element *self, int32_t index, double zStart, const vec_string *words); /* +0x28 */
    double (*getRadius)(Element *self, double z);                                         /* +0x30 */
};

struct Element {
    const ElementVtbl *vptr;/* 0x00                                               确定 */
    vec_string words;       /* 0x08 lattice 行拆出的原始单词                       确定 */
    int32_t index;          /* 0x20 元件序号                                      确定 */
    int32_t pad_24;         /* 0x24                                               */
    double  radius;         /* 0x28 孔径半径 R（m），getRadius 返回               确定 */
    double  radius2;        /* 0x30 = R 的副本（可能是 Ry）                       推测 */
    int32_t apertureType;   /* 0x38 置 0                                          推测 */
    int32_t pad_3c;         /* 0x3c                                               */
    cxx_string typeName;    /* 0x40 关键字（构造时为 "drift"）                    确定 */
    double  zStart;         /* 0x48 元件入口 z（m）                               确定 */
    double  length;         /* 0x50 L（m）                                        确定 */
    double  strength;       /* 0x58 场强：quad G(T/m)、solenoid B(T)；误差按 dG 缩放 确定 */
    double  unk_60;         /* 0x60                                               推测 */
    int32_t inSuperposeOut; /* 0x68 非 0 = 位于 superposeout 偏移坐标系中         推测 */
    int32_t unk_6c;         /* 0x6c 运行中被清 0 的标志                           推测 */
    vec_double unk_70;      /* 0x70 ~Element 释放                                 推测 */
};                          /* sizeof = 0x88（= Drift，InitLattice 中 operator new(0x88)）*/

typedef struct { Element base; } Drift;                         /* sizeof = 0x88 */

typedef struct Quadrupole {
    Element base;           /* 0x00 */
    double  unk_88;         /* 0x88 构造时 0                                      推测 */
    double  c90;            /* 0x90 构造时 -4e-5                                  推测（常数，含义不明）*/
    double  c98;            /* 0x98 构造时 4.518219                               推测 */
    double  aperDiam;       /* 0xa0 = 2R                                          确定 */
    double  unk_a8[3];      /* 0xa8                                               推测 */
    double  gradErr;        /* 0xc0 加误差后的 G                                  确定 */
    double  dx;             /* 0xc8 错位 m                                        确定 */
    double  dy;             /* 0xd0                                               确定 */
    double  dz;             /* 0xd8                                               确定 */
    double  rotX;           /* 0xe0 rad                                           确定 */
    double  rotY;           /* 0xe8                                               确定 */
    double  rotZ;           /* 0xf0                                               确定 */
} Quadrupole;               /* sizeof = 0xf8 */

typedef struct Solenoid {
    Element base;           /* 0x00 */
    double  fieldErr;       /* 0x88 加误差后的 B                                  确定 */
    double  dx, dy, dz;     /* 0x90 0x98 0xa0 m                                   确定 */
    double  rotX, rotY, rotZ; /* 0xa8 0xb0 0xb8 rad                               确定 */
} Solenoid;                 /* sizeof = 0xc0 */

typedef struct Steerer {
    Element base;           /* 0x00 */
    int32_t kind;           /* 0x88 0 磁 / 1 电（word6）                          确定 */
    int32_t pad_8c;         /* 0x8c                                               */
    double  bx;             /* 0x90 Bx 或 Ex（word4）                             确定 */
    double  by;             /* 0x98 By 或 Ey（word5）                             确定 */
    double  maxValue;       /* 0xa0 最大值（word7）                               确定 */
} Steerer;                  /* sizeof = 0xa8 */

typedef struct Bend {
    Element base;           /* 0x00 */
    int32_t hv;             /* 0x88 0 水平 / 1 竖直（word7）                      确定 */
    int32_t pad_8c;         /* 0x8c                                               */
    double  arcLength;      /* 0x90 ρ|α|（m）                                     确定 */
    double  alpha;          /* 0x98 偏转角（rad，word4）                          确定 */
    double  kx;             /* 0xa0 sqrt((1-n)h²)                                 确定 */
    double  ky;             /* 0xa8 sqrt(n h²)                                    确定 */
    double  rho;            /* 0xb0 曲率半径（m，word5）                          确定 */
    double  n;              /* 0xb8 场梯度指数（word6）                           确定 */
    double  h;              /* 0xc0 sign(α)/ρ（1/m）                              确定 */
    double  unk_c8;         /* 0xc8 GetParameter 会输出，但 SetParameter 不写     推测 */
    double  fieldErr;       /* 0xd0                                               确定 */
    double  dx, dy, dz;     /* 0xd8 0xe0 0xe8                                     确定 */
    double  rotX, rotY, rotZ; /* 0xf0 0xf8 0x100                                  确定 */
} Bend;                     /* sizeof = 0x108 */

typedef struct Edge {
    Element base;           /* 0x00 */
    int32_t hv;             /* 0x88 （word9）                                     确定 */
    int32_t pad_8c;         /* 0x8c                                               */
    double  beta;           /* 0x90 极面角（rad，word4）                          确定 */
    double  k1;             /* 0x98 （word7）                                     确定 */
    double  k2;             /* 0xa0 （word8）                                     确定 */
    double  rho;            /* 0xa8 |ρ|（word5）                                  确定 */
    double  gap;            /* 0xb0 G（word6）                                    确定 */
    double  psi;            /* 0xb8 边缘场修正角 ψ                                确定 */
    double  fieldErr;       /* 0xc0                                               确定 */
    double  dx, dy, dz;     /* 0xc8 0xd0 0xd8                                     确定 */
    double  rotX, rotY, rotZ; /* 0xe0 0xe8 0xf0                                   确定 */
} Edge;                     /* sizeof = 0xf8 */

typedef struct Field {
    Element base;           /* 0x000 */
    int32_t mapIndex;       /* 0x088 FieldMatrix 中的场图序号（word9 被替换后）   确定 */
    int32_t fieldType;      /* 0x08c 1 RF / 2 静电 / 3 静磁 / 4 叠加组（word4）   确定(1,4)/推测(2,3) */
    double  ke;             /* 0x090 （word7）                                    确定 */
    double  kb;             /* 0x098 （word8）                                    确定 */
    double  zEnd;           /* 0x0a0 zStart + L                                   确定 */
    double  frequency;      /* 0x0a8 Hz（word5）                                  确定 */
    double  errDx;          /* 0x0b0 m                                            确定 */
    double  errDy;          /* 0x0b8                                              确定 */
    double  errDz;          /* 0x0c0                                              确定 */
    double  errRotX;        /* 0x0c8 rad                                          确定 */
    double  errRotY;        /* 0x0d0                                              确定 */
    double  errRotZ;        /* 0x0d8（仅静场误差写）                              确定 */
    double  keErr;          /* 0x0e0 Ke·(1+dK%)                                   确定 */
    double  kbErr;          /* 0x0e8                                              确定 */
    double  phaseErr;       /* 0x0f0 相位误差（单位按输入，推测为 deg）           推测 */
    vec_double zeroField;   /* 0x0f8 {0,0,0}，越界时拷给粒子                      确定 */
    double  phase;          /* 0x110 相位（rad，word6）                           确定 */
    int32_t cavIndex;       /* 0x118 射频腔序号（InitLattice 追加的 word14）      确定 */
    int32_t pad_11c;        /* 0x11c                                              */
    vec_double subZStart;   /* 0x120 叠加子场入口 z                               确定 */
    vec_int    subType;     /* 0x138 叠加子场场类型                               确定 */
    vec_int    subMapIndex; /* 0x150 叠加子场场图序号                             确定 */
    vec_double subKe;       /* 0x168                                              确定 */
    vec_double subKb;       /* 0x180                                              确定 */
    vec_vec_double subPlace;/* 0x198 每个子场 {3 个位移, θ×3(rad)}                推测（顺序）*/
    vec_double subRadius;   /* 0x1b0 子场孔径                                     推测 */
    vec_double subLength;   /* 0x1c8 子场长度                                     推测 */
    vec_vec_double subErr;  /* 0x1e0 每个子场的误差参数                           推测 */
} Field;                    /* sizeof = 0x1f8 */

typedef struct RFQ {
    Element base;           /* 0x000 */
    vec_vec_double cellCoef;/* 0x088 每个 cell 的场展开系数                       推测 */
    vec_double cellLength;  /* 0x0a0 m（cm/100）                                  确定 */
    vec_double cellZStart;  /* 0x0b8                                              确定 */
    vec_double cellZEnd;    /* 0x0d0 getEleNum 按它查 cell                        确定 */
    vec_double cellAperture;/* 0x0e8 m，getRadius 返回                            确定 */
    vec_double cellVoltage; /* 0x100 读入值 ×1000                                 推测（kV→V）*/
    vec_double unk_118;     /* 0x118 每 cell 一个参数                             推测 */
    vec_double unk_130;     /* 0x130                                              推测 */
    int32_t cavIndex;       /* 0x148 （word14）                                   确定 */
    int32_t pad_14c;        /* 0x14c                                              */
    double  frequency;      /* 0x150 Hz（word5）                                  确定 */
    vec_double zeroField;   /* 0x158 {0,0,0}                                      确定 */
    double  eps;            /* 0x170 1e-16                                        确定 */
} RFQ;                      /* sizeof = 0x178 */

/* ================================================================== Lattice
 * sizeof：main_agent 栈数组 Lattice local_548[624] → 0x270；最后成员 0x250 vector → 0x268。
 */
struct Lattice {
    vec_element_ptr elements;   /* 0x000                                          确定 */
    FieldMatrix fm;             /* 0x018 （0x018–0x1d7）                          确定 */
    vec_double elemZEnd;        /* 0x1d8                                          确定 */
    vec_double elemZStart;      /* 0x1f0                                          确定 */
    int32_t spOutCur;           /* 0x208 主束当前 superposeout 变换序号           推测 */
    int32_t pad_20c;            /* 0x20c                                          */
    vec_vec_double spOut;       /* 0x210 superposeout 变换 {z0,x0,y0,θz,θx,θy,出口 z} 推测（顺序）*/
    int32_t spOutCount;         /* 0x228 superposeout 块数                        推测 */
    int32_t unk_22c;            /* 0x22c                                          推测 */
    vec_double elemLength;      /* 0x230                                          确定 */
    int32_t lastMapIndex;       /* 0x248 最近一次 SetMatrix 的返回值              确定 */
    int32_t pad_24c;            /* 0x24c                                          */
    vec_double unk_250;         /* 0x250 ~Lattice 释放，元素类型未知              推测 */
};                              /* sizeof = 0x268（栈上占 0x270）*/

/* ================================================================== Geometry / Boundary（boundary.txt）*/
struct Geometry {
    vec_string words;       /* 0x00                                               确定 */
    vec_double place;       /* 0x18 6 个：3 位移 + 3 角度（deg→rad）              推测 */
    double  zStart;         /* 0x30                                               确定 */
    int32_t index;          /* 0x38                                               确定 */
    int32_t pad_3c;         /* 0x3c                                               */
    vec_double shapePar;    /* 0x40 形状参数                                      推测 */
    int32_t shapeType;      /* 0x58 word0：0/1/2                                  确定 */
    int32_t pad_5c;         /* 0x5c                                               */
    cxx_string name;        /* 0x60 word1（如 "RLP"/"Copper"）                    推测 */
    int32_t unk_68;         /* 0x68 word4                                         推测 */
    int32_t unk_6c;         /* 0x6c word13                                        推测 */
    vec_double unk_70;      /* 0x70                                               推测 */
    vec_vec_double unk_88;  /* 0x88                                               推测 */
    vec_double unk_a0;      /* 0xa0                                               推测 */
    vec_int    unk_b8;      /* 0xb8                                               推测 */
    vec_double unk_d0;      /* 0xd0                                               推测 */
    vec_double unk_e8;      /* 0xe8 word6..12                                     推测 */
};                          /* sizeof = 0x100（operator new(0x100)）*/

struct Boundary {
    vec_geometry_ptr geoms; /* 0x00                                               确定 */
    vec_double zEnd;        /* 0x18                                               确定 */
    vec_double zStart;      /* 0x30                                               确定 */
    vec_double length;      /* 0x48                                               确定 */
    vec_vec_double spOutMap;/* 0x60 每个 superposeout 块 {z 偏移, 首几何段, 末几何段} 推测 */
};                          /* sizeof = 0x78 */

/* ================================================================== 误差 */
struct ErrorSample {
    vec_int flags;          /* 0x00 [0] 束流静态 [1] 束流动态 [2] 元件静态 [3] 元件动态 确定 */
    vec_vec_double dyn;     /* 0x18 行 0 = 束流误差，行 i+1 = 元件 i（DynamicErrorData.txt）确定 */
    vec_vec_double stat;    /* 0x30 同上（StaticErrorData.txt）                   确定 */
};                          /* sizeof = 0x48 */

struct ErrorParameter {     /* 只恢复了容器位置，含义见 structures.md            推测 */
    vec_int unk_00;                 /* 0x00 */
    vec_vec_vec_double unk_18;      /* 0x18 */
    vec_vec_vec_double unk_30;      /* 0x30 */
    vec_double errStep;             /* 0x48 err_step 等数值（首元素补 0）       推测 */
    vec_vec_double elemErr;         /* 0x60 每条 err_* 定义                     推测 */
    vec_int switches;               /* 0x78 误差开关，补足到 30 / 60 个          推测 */
    vec_vec_double unk_90;          /* 0x90 */
    int32_t unk_a8;                 /* 0xa8 */
    int32_t pad_ac;                 /* 0xac */
    vec_double unk_b0;              /* 0xb0 */
};                                  /* sizeof >= 0xc8 */

/* ================================================================== Mesh / PIC（只列关键成员）*/
struct Mesh {
    vec_int  n;                     /* 0x000 网格数 {Nx,Ny,Nz}                    确定 */
    int32_t  nParticles;            /* 0x018                                      确定 */
    int32_t  pad_01c;               /* 0x01c */
    vec_vec_int cellOf;             /* 0x020 每个粒子的格点下标（3 个 int）        确定 */
    vec_double xs;                  /* 0x038 Nx 个 x 网格坐标                     确定 */
    vec_double ys;                  /* 0x050                                      确定 */
    vec_double zs;                  /* 0x068                                      确定 */
    vec_vec_vec_double grid[8];     /* 0x080..0x128 8 个三维数组（电荷、势、场…）  推测（各自含义）*/
};                                  /* sizeof = 0x140（MethodMP::Run 栈上 Mesh[320]）*/

typedef struct { uint8_t raw[0x168]; } ProgschjThreadPool;  /* progschj::ThreadPool，0x168 字节  推测 */

struct PIC {
    ProgschjThreadPool pool;        /* 0x000                                      确定（~PIC 调 ~ThreadPool）*/
    int32_t unk_168, unk_16c;       /* 0x168 */
    double  unk_170;                /* 0x170 构造时 1.0 */
    uint8_t unk_178[0x1b8 - 0x178]; /* 0x178 */
    int32_t nx, ny, nz;             /* 0x1b8 网格数                               确定 */
    int32_t nx1, ny1, nz1;          /* 0x1c4 N-1                                  确定 */
    int32_t nx2, ny2, nz2;          /* 0x1d0 N/2                                  确定 */
    int32_t unk_1dc, unk_1e0;       /* 0x1dc */
    int32_t scPar9;                 /* 0x1e4 = SC 参数[9]                         推测 */
    int32_t unk_1e8;                /* 0x1e8 */
    int32_t pad_1ec;
    uint8_t unk_1f0[0x240 - 0x1f0]; /* 0x1f0 */
    double  meshRms[3];             /* 0x240 meshrms                              确定 */
    double  scMethod;               /* 0x258 0 = FFT，非 0 = SPICNIC              确定 */
    uint8_t unk_260[0x4d0 - 0x260]; /* 0x260 （0x2b0 vector<int> 网格数 等）      */
    void   *fftwBuf[8];             /* 0x4d0..0x508 fftw_malloc 缓冲              确定 */
    void   *fftwPlan[4];            /* 0x510..0x528 fftw_plan                     确定 */
};                                  /* sizeof >= 0x530 */

/* ================================================================== Initial（input.txt / beam.txt 读入）*/
struct Initial {
    int32_t secondaryBeam;          /* 0x000 secondarybeam                        确定 */
    int32_t singleParticleMode;     /* 0x004 singleparticlemode                   确定 */
    cxx_string distFile;            /* 0x008 readparticledistribution（默认 "unknown"）确定 */
    double  charge;                 /* 0x010 numofcharge（e）                     确定 */
    double  atomicMass;             /* 0x018 = mc²/938.271875                     确定 */
    vec_string distribution;        /* 0x020 {横向, 纵向}，默认 {"GS","GS"}       确定 */
    int32_t particleNumber;         /* 0x038 particlenumber                       确定 */
    int32_t unk_03c;                /* 0x03c 默认 -1                              推测 */
    vec_double twiss;               /* 0x040 [εx,βx,αx, εy,βy,αy, εz,βz,αz]       确定 */
    vec_double displacePos;         /* 0x058 displacepos（3）                     确定 */
    vec_double displacedPos;        /* 0x070 displacedpos（3）                    确定 */
    vec_int    cores;               /* 0x088 setnumberofcores                     确定 */
    double  restMass;               /* 0x0a0 particlerestmass MeV（默认 938.272029）确定 */
    double  current;                /* 0x0a8 current（读入 mA，InitParticle 后为 A）确定 */
    double  frequency;              /* 0x0b0 frequency（读入 MHz，InitParticle 后为 Hz）确定 */
    double  kineticEnergy;          /* 0x0b8 kneticenergy MeV                     确定 */
    double  unk_0c0;                /* 0x0c0                                      推测 */
    double  randomSeed;             /* 0x0c8 randomseed（以 double 存）           确定 */
    vec_double longLimits;          /* 0x0d0 longlimits（5）                      确定 */
    cxx_string kind;                /* 0x0e8 kindofparticle（默认 "proton"）      确定 */
    vec_double beamLength;          /* 0x0f0 beamlength（2）                      确定 */
    cxx_string method;              /* 0x108 method（默认 "MP"）                  确定 */
    int32_t spaceCharge;            /* 0x110 spacecharge（默认 1）                确定 */
    int32_t pad_114;
    vec_double scPar;               /* 0x118 20 个：[0]scmethod [1..3]meshrms [4..6]integralgrid
                                                [7]scutoffdistance [8]negligibleratio [10..]cores 确定 */
    int32_t unk_130;                /* 0x130 默认 1（=1 时 t-code 初始化）        推测 */
    int32_t pad_134;
    double  scanTStep;              /* 0x138 scantstep（默认 0.03）               确定 */
    int32_t unk_140;                /* 0x140 默认 3                               推测 */
    int32_t boundary;               /* 0x144 boundary                             确定 */
    int32_t lpBoundary;             /* 0x148 lpboundary                           确定 */
    int32_t probe;                  /* 0x14c probe                                确定 */
    int32_t indWeight;              /* 0x150 indweight                            确定 */
    int32_t indCharge;              /* 0x154 indcharge                            确定 */
    vec_double pcHistogram;         /* 0x158 pchistogram（6）                     确定 */
    double  stepPerCycle;           /* 0x170 steppercycle V1（默认 50）           确定 */
    int32_t dumpPeriodicity;        /* 0x178 dumpperiodicity                      确定 */
    int32_t unk_17c;                /* 0x17c 默认 -1                              推测 */
    int32_t runPeriod;              /* 0x180 runperiod（默认 100000）             确定 */
    int32_t pad_184;
    vec_int track;                  /* 0x188 track                                确定 */
    int32_t unk_1a0;                /* 0x1a0                                      推测 */
    int32_t pad_1a4;
    vec_double outputPlanes;        /* 0x1a8 输出面 z 列表 → MethodMP+0x220       推测 */
    int32_t needSynPhase;           /* 0x1c0 有 V3=0 的腔时置 1                   确定 */
    int32_t scSolverRecord;         /* 0x1c4 scsolverrecord                       确定 */
    vec_int numOfGrid;              /* 0x1c8 numofgrid（3）                       确定 */
    int32_t unk_1e0;                /* 0x1e0                                      推测 */
    int32_t bunchNumber;            /* 0x1e4 bunchnumber                          确定 */
    int32_t multithreading;         /* 0x1e8 multithreading（默认 1）             确定 */
    int32_t maxThreads;             /* 0x1ec maxthreads（默认 -1）                确定 */
    int32_t ompMaxThreads;          /* 0x1f0 omp_get_max_threads()                确定 */
    int32_t pad_1f4;
    vec_double unk_1f8;             /* 0x1f8                                      推测 */
    double  seStep;                 /* 0x210 steppercycle V2（默认 0.01）         确定 */
    int32_t recScField;             /* 0x218 recscfield                           确定 */
    int32_t impScField;             /* 0x21c impscfield                           确定 */
};                                  /* sizeof = 0x220（main_agent 栈上与 Lattice 间距）*/

/* ================================================================== MethodMP（只列关键成员）*/
struct MethodMP {
    ProgschjThreadPool pool;        /* 0x000 线程池（workers/tasks/mutex/3 个 condvar）确定 */
    int64_t startClock;             /* 0x168 system_clock::now()（ns）            确定 */
    double  wallTime;               /* 0x170 累计墙钟时间                          推测 */
    double  unk_178[4];             /* 0x178 */
    double  dt;                     /* 0x198 = 1/(f·steppercycle)（s）            确定 */
    int32_t spaceCharge;            /* 0x1a0                                      确定 */
    int32_t pad_1a4;
    vec_int track;                  /* 0x1a8                                      确定 */
    int32_t multithreading;         /* 0x1c0                                      确定 */
    int32_t dumpPeriodicity;        /* 0x1c4                                      确定 */
    int32_t unk_1c8;                /* 0x1c8                                      推测 */
    int32_t boundary;               /* 0x1cc                                      确定 */
    int32_t lpBoundary;             /* 0x1d0                                      确定 */
    int32_t pad_1d4;
    vec_double pcHistogram;         /* 0x1d8                                      确定 */
    vec_double longLimits;          /* 0x1f0                                      确定 */
    vec_int    cores;               /* 0x208                                      确定 */
    vec_double outputPlanes;        /* 0x220 最后一个元素用作进度分母              推测 */
    vec_double scPar;               /* 0x238                                      确定 */
    vec_int    numOfGrid;           /* 0x250                                      确定 */
    double  progressPercent;        /* 0x268                                      确定 */
    double  runTime;                /* 0x270                                      确定 */
    double  remainingTime;          /* 0x278                                      确定 */
    double  zStartRun;              /* 0x280 进度计算的起点 z                     确定 */
    double  unk_288;                /* 0x288 构造时 3.0                           推测 */
    double  unk_290;                /* 0x290                                      推测 */
    int32_t maxSecondaryCount;      /* 0x298 二次粒子相关的最大值记录             推测 */
    int32_t pad_29c;
    uint8_t unk_2a0[8];             /* 0x2a0 */
};                                  /* sizeof = 0x2a8（main_agent 栈数组 long[85]）推测 */

#endif /* AVAS_RECOVERED_STRUCTS_H */
