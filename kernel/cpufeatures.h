#ifndef __KERNEL_CPUFEATURES_H
#define __KERNEL_CPUFEATURES_H


/*
 * 该文件用于记录CPU的特征和指令集支持情况
 */



/*  开放指令集信息,方便模块查询指令集支持情况  */
extern struct Basic_set_struct Basic_set;
extern struct Extened_set_struct Extened_set;
extern struct EAX6_ECX0_EAX_set_struct EAX6_ECX0_EAX_set;
extern struct EAX7_ECX0_EBX_set_struct EAX7_ECX0_EBX_set;
extern struct EAX7_ECX0_ECX_set_struct EAX7_ECX0_ECX_set;
extern struct EAX7_ECX0_EDX_set_struct EAX7_ECX0_EDX_set;


/*  ECX拓展指令集  */
struct Extened_set_struct{
    unsigned int
        SSE3:1,         //流式SIMD扩展3
        PCLMULQDQ:1,    // 	PCLMULQDQ指令
        DTES64:1,       //64位调试保存
        MINOTOR:1,      //MONITOR和MWAIT指令
        DS_CPL:1,       //
        VMX:1,          //虚拟机扩展
        SMX:1,          //安全模式扩展
        EST:1,          //增强SpeedStep
        TM2:1,          //温度传感器2
        SSSE3:1,        //流式SIMD扩展3补充指令集(Supplemental)
        CNXT_ID:1,      //一级缓存上下文ID
        SDBG:1,         //芯片调试接口
        FMA:1,          //3操作数融合乘加
        CX16:1,         //CMPXCHG16B指令
        XTPR:1,         //可以禁用发送任务优先级消息
        PDCM:1,         //
        res:1,          //保留
        PCID:1,         //进程上下文标识符
        DCA:1,          //DMA写入的直接缓存访问
        SSE4_1:1,       //流式SIMD扩展4.1
        SSE4_2:1,       //流式SIMD扩展4.2
        X2APIC:1,       //x2APIC支持
        MOVBE:1,        //MOVBE指令
        POPCNT:1,       //popcnt指令
        TSC_DEADLINE:1, //APIC 使用 TSC 截止时间值实现一次性操作
        AES:1,          //AES指令集
        XSAVE:1,        // 	XSAVE, XRESTOR, XSETBV, XGETBV指令
        OSXSAVE:1,      //操作系统启用XSAVE
        AVX:1,          //高级矢量扩展
        F16C:1,         //F16C指令集
        RDRND:1,        //集成随机数发生器
        HYPERVISOR:1;   //当前为虚拟机环境
};





/*  EDX基础指令集  */
struct Basic_set_struct{
    unsigned int
        FPU:1,      //集成FPU
        VME:1,      //虚拟8086模式
        DE:1,       //调试模式
        PSE:1,      //页尺寸拓展
        TSC:1,      //时钟周期计数器
        MSR:1,      //特殊模块寄存器
        PAE:1,      //物理地址拓展
        MCE:1,      //机器检查异常
        CX8:1,      //CMPXCHG8指令
        APIC:1,     //集成高级可编程中断控制器
        res_1:1,    //保留
        SEP:1,      //sysenter和sysexit指令支持
        MTRR:1,     //内存类型范围寄存器
        PGE:1,      //全局分页控制位
        MCA:1,      //机器检查架构
        CMOV:1,     //条件传送指令
        PAT:1,      //页属性表
        PSE_36:1,   //36位页尺寸拓展
        PSN:1,      //处理器序列号
        CLFSH:1,    //CLFUSH指令
        res_2:1,    //保留
        DS:1,       //调试保存
        ACPI:1,     //集成ACPI温控寄存器
        MMX:1,      //多媒体拓展
        FXSR:1,     //fxsave,fxstore指令
        SSE:1,      //流式SIMD拓展
        SSE2:1,     //流式SIMD拓展2
        SS:1,       //自窥探缓存
        HTT:1,      //超线程(Hyper-threading)
        TM:1,       //自控温温度传感器
        IA64:1,     //基于IA64架构的处理器
        PBE:1;      //挂起中断
};




/*  处理器的电源管理功能和性能状态信息  */
struct EAX6_ECX0_EAX_set_struct{
    unsigned int
        DIGITAL_TEMP_SENSOR:1,  //支持数字温度传感器
        TURBO_BOOST:1,          //Intel Turbo Boost
        ARAT:1,                 //支持始终运行的 APIC 定时器
        res_1:1,                //保留
        PLN:1,                  //支持电源限制通知
        ECMD:1,                 //支持时钟调制占空比扩展
        PTM:1,                  //支持封装热管理
        HWP:1,                  //支持HWP基础寄存器
        HWP_Notification:1,     //支持 IA32_HWP_INTERRUPT MSR
        HWP_ACTIVITY_WINDOW:1,  //支持 HWP 活动窗口
        HWP_EPPS:1,             //支持HWP能源性能偏好
        HWP_PLRS:1,             //支持HWP包级请求
        res_2:1,                //保留
        HDC:1,                  //支持HDC
        TURBO_BOOST_3:1,        //支持 Intel Turbo Boost 最大技术 3.0
        HWP_HP:1,               //支持最高性能变更
        HWP_PECI:1,             //支持HWP PECI覆盖
        FLEXIBLE_HWP:1,         //灵活的HWP
        FAST_HWP_MSR:1,         //支持 IA32_HWP_REQUEST MSR 的快速访问模式
        HW_FEEDBACK:1,          //支持硬件反馈
        IGNORE_IDLE_HWP:1,      //支持忽略空闲逻辑处理器的 HWP 请求
        res_3:1,                //保留
        THREAD_DIRECTOR:1,      //支持 Intel线程指导器
        THERM_INTERRUPT_MSR:1,  // 支持 IA32_THERM_INTERRUPT MSR 第25位
        res_4:7;
} ;





struct EAX6_ECX0_ECX_set_struct{
    unsigned int
        Hardware_Coordination_Feedback:1,       //硬件调节反馈能力
        res_1:2,        //保留
        Performance_energy:1,       //支持性能-能源偏好设置
        res_2:4,        //保留
        Number_Thread_Director:8,       //Intel Thread DIrector类别数量
        res:16;
};







struct EAX7_ECX0_EBX_set_struct {
    unsigned int
        FSGSBASE:1,         //RDFSBASE等指令
        IA32_TSC_ADJUST:1,  //支持 IA32_TSC_ADJUST MSR
        SGX:1,              //支持 Intel软件防护扩展
        BMI1:1,             //支持BMI1
        HLE:1,              //支持硬件锁定强化
        AVX2:1,             //支持Intel高级向量扩展2
        FDP_EXCPTN_ONLY:1,  //x87 FPU 数据指针仅在 x87 异常时更新
        SMEP:1,             //支持监管模式执行防护
        BMI2:1,             //支持 BMI2
        REP_MOVSB:1,        //支持增强的 REP MOVSB/STOSB 指令
        INVPCID:1,          //支持 INVPCID 指令，用于管理进程上下文标识符
        RTM:1,              //支持 RTM
        RDT_M:1,            //支持 Intel资源导向技术监控功能
        FPU_CS_DS:1,        //弃用FPU CS和DS值
        MPX:1,              //支持Intel内存保护扩展
        RDT_A:1,            //支持资源导向技术分配功能
        AVX512F:1,          //支持 AVX512F
        AVX512DQ:1,         //支持 AVX512DQ
        RDSEED:1,           //支持 RDSEED
        ADX:1,              //支持 ADX
        SMAP:1,             //支持监管模式访问防护（SMAP）及 CLAC/STAC 指令
        AVX512_IFMA:1,      //支持 AVX512_IFMA
        res_1:1,            //保留
        CLFLUSHOPT:1,       //支持 CLFLUSHOPT
        CLWB:1,             //支持 CLWB
        TRACE:1,            //支持 Intel 处理器跟踪
        AVX512PF:1,         //仅在 Intel® Xeon PhiTM 中支持 AVX512PF
        AVX512ER:1,         //仅在 Intel Xeon PhiTM 中支持 AVX512ER
        AVX512CD:1,         //支持 AVX512CD
        SHA:1,              //支持Intel安全哈希算法扩展
        AVX512BW:1,         //支持AVX512BW
        AVX512VL:1;         //支持AVX512VL
};



struct EAX7_ECX0_ECX_set_struct{
    unsigned int
        	PREFETCHWT1:1,     //仅在 Intel Xeon Phi™ 中支持 PREFETCHWT1 指令
            AVX512_VBMI:1,      //支持 AVX-512 向量位操作指令集 VMBI
            UMIP:1,             //支持用户模式指令防护（UMIP）
            PKU:1,              //支持用户模式页面的保护密钥
            OSPKE:1,            //操作系统已设置CR4，PKE启用保护密钥
            WAITPKG:1,          //支持 WAITPKG 指令
            AVX512_VBMI2:1,     //支持 AVX-512 向量位操作指令集 VMBI2
            CET_SS:1,           //支持 CET 阴影栈特性
            GFNI:1,             //支持 GFNI 指令集
            VAES:1,             //支持AES指令集
            VPCLMULQDQ:1,       //支持向量化的 PCLMULQDQ 指令
            AVX512_VNNI:1,      //支持 AVX-512 向量神经网络指令
            AVX512_BITALG:1,    //支持 AVX-512 位算法指令
            TME_EN:1,           //支持总内存加密
            AVX512_VPOPCNTDQ:1, //支持 AVX-512 VPOPCNTDQ 指令
            res_1:1,            //保留
            LA57:1,             //支持 57 位线性地址和五级分页
            MAWAU:1,            //BNDLDX 和 BNDSTX 指令在 64 位模式下使用的 MAWAU值

            RDPID:1,            //支持 RDPID 和 IA32_TSC_AUX
            KL:1,               //支持 Key Locker
            BUS_LOCK_DETECT:1, //支持操作系统总线锁检测
            CLDEMOTE:1,    	    //支持缓存行降级
            res_2:1,            //保留
            MOVDIRI:1,          //支持 MOVDIRI 指令
            MOVDIR64B:1,        //支持 MOVDIR64B 指令
            ENQCMD:1,           //支持 Enqueue Stores 指令
            SGX_LC:1,           //支持SGX启动配置
            PKG:1;              //支持监管模式页面的保护魔药
};



struct EAX7_ECX0_EDX_set_struct{
    unsigned int
        res_1:1,
        SGX_KEYS:1,     //支持SGX的认证服务
        AVX512_4VNNIW:1,    //支持 AVX512_4VNNIW
        AVX512_4FMAPS:1,    //支持 AVX512_4FMAPS
        Fast_Short_REP_MOV:1,   //快速短REP MOVBE指令
        UINTRL:1,           //支持用户中断
        res_2:2,
        AVX512_VP2INTERSECT:1,  //支持 AVX512 VP2INTERSECT 指令
        SRBDS_CTRL:1,       //支持 IA32_MCU_OPT_CTRL MSR 并指示支持其 RNGDS_MITG_DIS 位
        MD_CLEAR:1,   //支持 MD_CLEAR
        RTM_ALWAYS_ABORT:1, //任何 XBEGIN 执行立即中止，并转到指定的回退地址
        res_3:1,
        RTM_FORCE_ABORT:1,  //支持 RTM_FORCE_ABORT
        SERIALIZE:1,        //	支持 SERIALIZE 指令
        Hybrid:1,       //识别处理器为混合型部件
        TSXLDTRK:1,     //处理器支持 Intel TSX 的负载地址跟踪的挂起/恢复
        res_4:1,
        PCONFIG:1,      //支持 PCONFIG 指令
        Architectural_LBRs:1,   //支持架构级的最近分支记录
        CET_IBT:1,  //支持CET间接分支跟踪特性
        res_5:1,
        AVX_BF16:1,     //在 bfloat16 数字上的瓦片计算操作
        AVX512_FP16:1,  //支持 AVX-512 FP16 指令集
        AMX_TILE:1,     //支持瓦片架构
        AMX_INTB:1,     //在8位整数上的瓦片计算操作
        IBRS_IBPB:1,    //间接分支限制猜测\间接分支预测器屏障
        STIBP:1,        //支持单线程间接分支预测器
        L1D_FLUSH:1,    //支持 L1D_FLUSH 命令
        IA32_ARCH_CAPABILITIES:1,
        IA32_CORE_CAPABILITIES :1,
        SSBD:1;       //Speculative Store Bypass Disable
};



#endif