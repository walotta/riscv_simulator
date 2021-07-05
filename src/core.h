//
// Created by wzj on 2021/6/29.
//

#ifndef RISCV_SIMULATOR_CORE_H
#define RISCV_SIMULATOR_CORE_H

#include "memory.h"
#include "register.h"
#include <iomanip>

class core
{
private:
    /*class environment
    {
    public:
        unsigned int cmd{};
        unsigned int imm{};
        unsigned int rd{};
        unsigned int rs1=0;
        unsigned int rs2=0;
        unsigned int shamt{};
        unsigned int funct3{},funct7{};
        unsigned int opcode;
        unsigned int wb_value=0;
        unsigned int mem_operator_address=0;
        char type{};
        unsigned int pc{};
        void allClear()
        {
            cmd=imm=rd=rs1=rs2=shamt=funct3=funct7=opcode=wb_value=mem_operator_address=pc=0;
            type=char(0);
        }
        void clearDate()
        {
            cmd=imm=rd=rs1=rs2=shamt=funct3=funct7=opcode=wb_value=mem_operator_address=0;
            type=char(0);
        }
        environment& operator=(const environment& o)
        {
            if(this==&o)return *this;
            cmd=o.cmd;
            imm=o.imm;
            rd=o.rd;
            rs1=o.rs1;
            rs2=o.rs2;
            shamt=o.shamt;
            funct3=o.funct3;
            funct7=o.funct7;
            opcode=o.opcode;
            wb_value=o.wb_value;
            mem_operator_address=o.mem_operator_address;
            type=o.type;
            pc=o.pc;
            return *this;
        }
    };*/

    struct if_id
    {
        unsigned int cmd{};
        unsigned int pc{};
        void clear()
        {
            pc=cmd=0;
        }
    };

    struct id_ex
    {
        unsigned int pc{};
        unsigned int cmd{};
        unsigned int opcode{};
        char type{};
        unsigned int imm{};
        unsigned int rd{};
        unsigned int rs1=0;
        unsigned int rs2=0;
        unsigned int shamt{};
        unsigned int funct3{},funct7{};
        bool jump_cmd=false;
        void clear()
        {
            pc=cmd=opcode=imm=rd=rs1=rs2=shamt=funct3=funct7=0;
            type=0;
            jump_cmd=false;
        }
    };

    struct ex_mem
    {
        unsigned int opcode{};
        unsigned int funct3{};
        unsigned int funct7{};
        unsigned int wb_value{};
        unsigned int rs1{};
        unsigned int rs2{};
        unsigned int rd{};
        unsigned int mem_operator_address{};
        unsigned int pc{};
        unsigned int cmd{};
        bool has_jump=false;
        void clear()
        {
            opcode=cmd=funct3=funct7=wb_value=rs1=rs2=rd=mem_operator_address=pc=0;
            has_jump=false;
        }
    };

    struct mem_wb
    {
        unsigned int pc{};
        unsigned int cmd{};
        unsigned int opcode{};
        unsigned int funct3{},funct7{};
        unsigned int rd{};
        unsigned int wb_value{};
        void clear()
        {
            cmd=opcode=funct3=funct7=rd=wb_value=0;
        }
    };

    void expend_with_symbol(unsigned int& to_expend,int max_pos)
    {
        if(to_expend&((unsigned int)1<<max_pos))
        {
            for(int i=max_pos+1;i<32;i++)
                to_expend+=((unsigned int)1<<i);
        }
    }

    std::pair<bool,std::pair<int,int>>read_reg(unsigned int rs1,unsigned int rs2)
    {
        unsigned int list[2]={rs1,rs2};
        unsigned int ans[2];
        for(int i=0;i<2;i++)
        {
            if(list[i]==0)ans[i]=0;
            else if(runReg.mirror_mem_run&&list[i]==runReg.mirror_mem_to_wb.rd)
            {
                return std::make_pair(false,std::make_pair(0,0));
            }else if(runReg.mirror_ex_run&&list[i]==runReg.mirror_ex_to_mem.rd)
            {
                return std::make_pair(false,std::make_pair(0,0));
            }else if(runReg.mirror_id_run&&list[i]==runReg.mirror_id_to_ex.rd)
            {
                return std::make_pair(false,std::make_pair(0,0));
            }else
            {
                ans[i]=reg_living.read(list[i]);
            }
        }
        return std::make_pair(true,std::make_pair(ans[0],ans[1]));
    }

    enum reg_use{available,in_wb,in_mem,in_ex,in_id};
    struct WAIT_CLOCK
    {
        int IF=0;
        int ID=0;
        int EX=0;
        int MEM=0;
        int WB=0;
        bool simple_load=false;
        reg_use reg_statue[32]{};
    }waitClock_mirror,waitClock;
    int clock_cnt=0;

    void IF(const unsigned int& pc,if_id& out)
    {
        //std::cerr<<std::endl<<" IF "<<'@'<<std::hex<<std::setw(4)<<std::setfill('0')<<pc<<std::endl;
        if(waitClock_mirror.IF!=0)
        {
            waitClock.IF--;
            runReg.if_run=false;
            runReg.pc-=4;
            //std::cerr<<"block"<<std::endl;
        }
        else
        {
            //std::cerr<<"finish"<<std::endl;
            runReg.if_run=true;
            out.clear();
            out.cmd=0;
            out.pc=pc;
            for(int i=0;i<4;i++)
            {
                out.cmd<<=8;
                out.cmd+=mem_living[pc+3-i];
            }
        }
    }

    void ID(const if_id& in,id_ex& out)
    {
        //std::cerr<<std::endl<<" ID "<<'@'<<std::hex<<std::setw(4)<<std::setfill('0')<<in.pc<<std::endl;
        if(waitClock_mirror.ID!=0)
        {
            waitClock.ID--;
            //std::cerr<<"block"<<std::endl;
            runReg.id_run=false;
            if(waitClock_mirror.simple_load)
            {
                auto reg_ans=read_reg(out.rs1,out.rs2);
                if(reg_ans.first)
                {
                    runReg.id_run=true;
                    waitClock.simple_load=false;
                    //waitClock.IF++;
                    runReg.run_with_out_if=true;
                    out.rs1=reg_ans.second.first;
                    out.rs2=reg_ans.second.second;
                    //std::cerr<<"reg read"<<std::endl;
                }else
                {
                    waitClock.IF++;
                    waitClock.ID++;
                    waitClock.EX++;
                    waitClock.simple_load=true;
                    //std::cerr<<"reg read block"<<std::endl;
                }
            }
        }
        else
        {
            if(!runReg.mirror_if_run&&!runReg.run_with_out_if)
            {
                //std::cerr<<"block"<<std::endl;
                runReg.id_run=false;
                return;
            }
            runReg.run_with_out_if=false;
            runReg.id_run=true;
            out.clear();
            out.opcode=(unsigned int)0b01111111&in.cmd;
            out.pc=in.pc;
            out.cmd=in.cmd;
            bool need_r1=false;
            bool need_r2=false;
            switch(out.opcode)
            {
                case (unsigned int)0b0110111:
                    out.type='U';
                    break;
                case (unsigned int)0b0010111:
                    out.type='U';
                    break;
                case (unsigned int)0b1101111:
                    out.type='J';
                    break;
                case (unsigned int)0b1100111:
                    out.type='I';
                    break;
                case (unsigned int)0b1100011:
                    out.type='B';
                    break;
                case (unsigned int)0b0000011:
                    out.type='I';
                    break;
                case (unsigned int)0b0100011:
                    out.type='S';
                    break;
                case (unsigned int)0b0010011:
                    out.type='I';
                    break;
                case (unsigned int)0b0110011:
                    out.type='R';
                    break;
            }
            switch(out.type)
            {
                case 'R':
                    out.rd=(in.cmd&((unsigned int)0b11111<<7))>>7;
                    out.funct3=(in.cmd&((unsigned int)0b111<<12))>>12;
                    out.rs1=(in.cmd&((unsigned int)0b11111<<15))>>15;
                    out.rs2=(in.cmd&((unsigned int)0b11111<<20))>>20;
                    out.funct7=(in.cmd&((unsigned int)0b1111111<<25))>>25;
                    need_r1=true;
                    need_r2=true;
                    break;
                case 'I':
                    out.rd=(in.cmd&((unsigned int)0b11111<<7))>>7;
                    out.funct3=(in.cmd&((unsigned int)0b111<<12))>>12;
                    out.rs1=(in.cmd&((unsigned int)0b11111<<15))>>15;
                    out.imm=(in.cmd&((unsigned int)0b111111111111<<20))>>20;
                    expend_with_symbol(out.imm,11);
                    need_r1=true;
                    break;
                case 'S':
                    out.funct3=(in.cmd&((unsigned int)0b111<<12))>>12;
                    out.rs1=(in.cmd&((unsigned int)0b11111<<15))>>15;
                    out.rs2=(in.cmd&((unsigned int)0b11111<<20))>>20;
                    out.imm=0;
                    out.imm+=((in.cmd&((unsigned int)0b11111<<7))>>7);
                    out.imm+=((in.cmd&((unsigned int)0b1111111<<25))>>25)<<5;
                    expend_with_symbol(out.imm,11);
                    need_r1=true;
                    need_r2=true;
                    break;
                case 'B':
                    out.funct3=(in.cmd&((unsigned int)0b111<<12))>>12;
                    out.rs1=(in.cmd&((unsigned int)0b11111<<15))>>15;
                    out.rs2=(in.cmd&((unsigned int)0b11111<<20))>>20;
                    out.imm=0;
                    out.imm+=((in.cmd&((unsigned int)0b1<<7))>>7)<<11;
                    out.imm+=((in.cmd&((unsigned int)0b1111<<8))>>8)<<1;
                    out.imm+=((in.cmd&((unsigned int)0b111111<<25))>>25)<<5;
                    out.imm+=((in.cmd&((unsigned int)0b1<<31))>>31)<<12;
                    expend_with_symbol(out.imm,12);
                    need_r1=true;
                    need_r2=true;
                    break;
                case 'U':
                    out.rd=(in.cmd&((unsigned int)0b11111<<7))>>7;
                    out.imm=((in.cmd&((unsigned int)0b11111111111111111111<<12))>>12)<<12;
                    expend_with_symbol(out.imm,31);
                    break;
                case 'J':
                    out.rd=(in.cmd&((unsigned int)0b11111<<7))>>7;
                    out.imm=0;
                    out.imm+=((in.cmd&((unsigned int)0b11111111<<12))>>12)<<12;
                    out.imm+=((in.cmd&((unsigned int)0b1<<20))>>20)<<11;
                    out.imm+=((in.cmd&((unsigned int)0b1111111111<<21))>>21)<<1;
                    out.imm+=((in.cmd&((unsigned int)0b1<<31))>>31)<<20;
                    expend_with_symbol(out.imm,20);
                    break;
            }
            if(out.opcode==(unsigned int)0b0010011&&(out.funct3==0b001||out.funct3==0b101))
            {
                out.shamt=(in.cmd&((unsigned int)0b11111<<20))>>20;
                out.funct7=(in.cmd&((unsigned int)0b1111111<<25))>>25;
                out.imm=0;
            }

            switch(out.opcode)
            {
                case (unsigned int)0b1101111:
                    //JAL
                    out.jump_cmd=true;
                    break;
                case (unsigned int)0b1100111:
                    //JALR
                    out.jump_cmd=true;
                    break;
                case (unsigned int)0b1100011:
                    switch(out.funct3)
                    {
                        case 0b000:
                            //BEQ
                            out.jump_cmd=true;
                            break;
                        case 0b001:
                            //BNE
                            out.jump_cmd=true;
                            break;
                        case 0b100:
                            //BLT
                            out.jump_cmd=true;
                            break;
                        case 0b101:
                            //BGE
                            out.jump_cmd=true;
                            break;
                        case 0b110:
                            //BLTU
                            out.jump_cmd=true;
                            break;
                        case 0b111:
                            //BGEU
                            out.jump_cmd=true;
                            break;
                    }
                    break;
            }
            if(out.jump_cmd)
            {
                waitClock.IF++;
                waitClock.ID++;
            }

            //read reg
            /*if(waitClock_mirror.wb_writing&&(need_r1||need_r2)&&((need_r1&&waitClock_mirror.wb_tar==out.rs1)||(need_r2&&waitClock_mirror.wb_tar==out.rs2)))
            {
                waitClock.IF++;
                waitClock.ID++;
                waitClock.EX++;
                waitClock.simple_load=true;
                //std::cerr<<"reg read block"<<std::endl;
            }else
            {
                out.rs1=reg_living.read(out.rs1);
                out.rs2=reg_living.read(out.rs2);
                //std::cerr<<"finish"<<std::endl;
            }*/
            auto reg_ans=read_reg(out.rs1,out.rs2);
            if(reg_ans.first)
            {
                out.rs1=reg_ans.second.first;
                out.rs2=reg_ans.second.second;
                //std::cerr<<"finish"<<std::endl;
            }else
            {
                waitClock.IF++;
                waitClock.ID++;
                waitClock.EX++;
                waitClock.simple_load=true;
                runReg.id_run=false;
                //std::cerr<<"reg read block"<<std::endl;
            }
        }

    }

    void EX(const id_ex& in,ex_mem& out)
    {
        //std::cerr<<std::endl<<" EX "<<'@'<<std::hex<<std::setw(4)<<std::setfill('0')<<in.pc<<std::endl;
        if(waitClock_mirror.EX!=0)
        {
            waitClock.EX--;
            //std::cerr<<"block"<<std::endl;
            runReg.ex_run=false;
        }
        else
        {
            if(!runReg.mirror_id_run)
            {
                //std::cerr<<"block"<<std::endl;
                runReg.ex_run=false;
                return;
            }
            runReg.ex_run=true;
            out.clear();
            out.pc=in.pc;
            out.opcode=in.opcode;
            out.funct3=in.funct3;
            out.funct7=in.funct7;
            out.rs1=in.rs1;
            out.rs2=in.rs2;
            out.rd=in.rd;
            out.cmd=in.cmd;
            switch(in.opcode)
            {
                case (unsigned int)0b0110111:
                    //LUI
                    out.wb_value=(in.imm);
                    break;
                case (unsigned int)0b0010111:
                    //AUIPC
                    out.wb_value=(in.imm)+in.pc;
                    break;
                case (unsigned int)0b1101111:
                    //JAL
                    out.wb_value=in.pc+4;
                    out.pc+=in.imm;
                    out.has_jump=true;
                    break;
                case (unsigned int)0b1100111:
                    //JALR
                    out.wb_value=in.pc+4;
                    out.pc=(in.imm+in.rs1)&~1;
                    out.has_jump=true;
                    break;
                case (unsigned int)0b1100011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //BEQ
                            if(in.rs1==in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                        case 0b001:
                            //BNE
                            if(in.rs1!=in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                        case 0b100:
                            //BLT
                            if((int)in.rs1<(int)in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                        case 0b101:
                            //BGE
                            if((int)in.rs1>=(int)in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                        case 0b110:
                            //BLTU
                            if(in.rs1<in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                        case 0b111:
                            //BGEU
                            if(in.rs1>=in.rs2)
                            {
                                out.pc+=in.imm;
                                out.has_jump=true;
                            }
                            break;
                    }
                    break;
                case (unsigned int)0b0000011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //LB
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b001:
                            //LH
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b010:
                            //LW
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b100:
                            //LBU
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b101:
                            //LHU
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                    }
                    break;
                case (unsigned int)0b0100011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //SB
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b001:
                            //SH
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                        case 0b010:
                            //SW
                            out.mem_operator_address=in.rs1+in.imm;
                            break;
                    }
                    break;
                case (unsigned int)0b0010011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //ADDI
                            out.wb_value=in.rs1+in.imm;
                            break;
                        case 0b001:
                            //SLLI
                            out.wb_value=(in.rs1)<<(in.shamt);
                            break;
                        case 0b010:
                            //SLTI
                            out.wb_value=(unsigned int)((int)in.rs1<(int)in.imm);
                            break;
                        case 0b011:
                            //SLTIU
                            out.wb_value=(unsigned int)(in.rs1<in.imm);
                            break;
                        case 0b100:
                            //XORI
                            out.wb_value=in.rs1 xor in.rd;
                            break;
                        case 0b101:
                            //SRLI||SRAI
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //SRLI
                                    out.wb_value=in.rs1>>in.shamt;
                                    break;
                                case 0b0100000:
                                    //SRAI
                                    bool flag=false;
                                    if(in.rs1&(1<<31))flag=true;
                                    out.wb_value=in.rs1>>in.shamt;
                                    if(flag)
                                    {
                                        for(int i=0;i<in.shamt;i++)
                                        {
                                            out.wb_value+=(1<<(31-i));
                                        }
                                    }
                                    break;
                            }
                            break;
                        case 0b110:
                            //ORI
                            out.wb_value=in.rs1|in.imm;
                            break;
                        case 0b111:
                            //ANDI
                            out.wb_value=in.rs1&in.imm;
                            break;
                    }
                    break;
                case (unsigned int)0b0110011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //ADD
                                    out.wb_value=in.rs1+in.rs2;
                                    break;
                                case 0b0100000:
                                    //SUB
                                    out.wb_value=in.rs1-in.rs2;
                                    break;
                            }
                            break;
                        case 0b001:
                            //SLL
                            out.wb_value=in.rs1<<(in.rs2&0b11111);
                            break;
                        case 0b010:
                            //SLT
                            out.wb_value=((int)in.rs1<(int)in.rs2);
                            break;
                        case 0b011:
                            //SLTU
                            out.wb_value=(in.rs1<in.rs2);
                            break;
                        case 0b100:
                            //XOR
                            out.wb_value=in.rs1 xor in.rs2;
                            break;
                        case 0b101:
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //SRL
                                    out.wb_value=in.rs1>>(in.rs2&0b11111);
                                    break;
                                case 0b0100000:
                                    //SRA
                                    out.wb_value=in.rs1>>(in.rs2&0b11111);
                                    if(in.rs1&(1<<31))
                                    {
                                        for(int i=0;i<(in.rs2&0b11111);i++)
                                        {
                                            out.wb_value+=(1<<(31-i));
                                        }
                                    }
                                    break;
                            }
                            break;
                        case 0b110:
                            //OR
                            out.wb_value=in.rs1|in.rs2;
                            break;
                        case 0b111:
                            //AND
                            out.wb_value=in.rs1&in.rs2;
                            break;
                    }
                    break;
            }
            if(in.jump_cmd&&in.pc!=out.pc)
            {
                waitClock.ID++;
                waitClock.EX+=2;
                runReg.pc=out.pc;
                out.pc=in.pc;
            }else
                runReg.run_with_out_if=true;
            //std::cerr<<"finish"<<std::endl;
        }
    }

    void MEM(const ex_mem& in,mem_wb& out)
    {
        //std::cerr<<std::endl<<"MEM "<<'@'<<std::hex<<std::setw(4)<<std::setfill('0')<<in.pc<<std::endl;
        if(waitClock_mirror.MEM!=0)
        {
            waitClock.MEM--;
            //std::cerr<<"block"<<std::endl;
            runReg.mem_run=false;
        }
        else
        {
            if(!runReg.mirror_ex_run)
            {
                //std::cerr<<"block"<<std::endl;
                runReg.mem_run=false;
                return;
            }
            runReg.mem_run=true;
            out.clear();
            out.opcode=in.opcode;
            out.funct3=in.funct3;
            out.funct7=in.funct7;
            out.rd=in.rd;
            out.wb_value=in.wb_value;
            out.pc=in.pc;
            out.cmd=in.cmd;
            switch(in.opcode)
            {
                case (unsigned int)0b0000011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //LB
                            out.wb_value=mem_living.read(in.mem_operator_address);
                            expend_with_symbol(out.wb_value,7);
                            break;
                        case 0b001:
                            //LH
                        {
                            unsigned int tem;
                            tem=mem_living.read(in.mem_operator_address+1);
                            tem<<=8;
                            tem+=mem_living.read(in.mem_operator_address);
                            expend_with_symbol(tem,15);
                            out.wb_value=tem;
                        }
                            break;
                        case 0b010:
                            //LW
                        {
                            unsigned int tem=0;
                            for(int i=3;i>=0;i--)
                            {
                                tem<<=8;
                                tem+=mem_living.read(in.mem_operator_address+i);
                            }
                            out.wb_value=tem;
                        }
                            break;
                        case 0b100:
                            //LBU
                            out.wb_value=mem_living.read(in.mem_operator_address);
                            break;
                        case 0b101:
                            //LHU
                            unsigned int tem;
                            tem=mem_living.read(in.mem_operator_address+1);
                            tem<<8;
                            tem+=mem_living.read(in.mem_operator_address);
                            out.wb_value=tem;
                            break;
                    }
                    break;
                case (unsigned int)0b0100011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //SB
                            mem_living.write(in.mem_operator_address, in.rs2&0b11111111);
                            break;
                        case 0b001:
                            //SH
                            mem_living.write(in.mem_operator_address, in.rs2&0b11111111);
                            mem_living.write(in.mem_operator_address+1, ((in.rs2>>8)&(unsigned int)0b11111111));
                            break;
                        case 0b010:
                            //SW
                            for(int i=0;i<4;i++)
                            {
                                mem_living.write(in.mem_operator_address+i, (in.rs2>>(8*i))&((unsigned int)0b11111111));
                            }
                            break;
                    }
                    break;
            }
            if(waitClock_mirror.EX<0)
                waitClock.MEM=-1;
            //std::cerr<<"finish"<<std::endl;
        }

    }

    void WB(const mem_wb& in)
    {
        //std::cerr<<std::endl<<" WB "<<'@'<<std::hex<<std::setw(4)<<std::setfill('0')<<in.pc<<std::endl;
        if(waitClock_mirror.WB!=0)
        {
            waitClock.WB--;
            //std::cerr<<"block"<<std::endl;
        }
        else
        {
            if(!runReg.mirror_mem_run)
            {
                //std::cerr<<"block"<<std::endl;
                return;
            }
            switch(in.opcode)
            {
                case (unsigned int)0b0110111:
                    //LUI
                    reg_living.write(in.rd,in.wb_value);
                    break;
                case (unsigned int)0b0010111:
                    //AUIPC
                    reg_living.write(in.rd,in.wb_value);
                    break;
                case (unsigned int)0b1101111:
                    //JAL
                    reg_living.write(in.rd,in.wb_value);
                    break;
                case (unsigned int)0b1100111:
                    //JALR
                    reg_living.write(in.rd,in.wb_value);
                    break;
                case (unsigned int)0b0000011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //LB
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b001:
                            //LH
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b010:
                            //LW
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b100:
                            //LBU
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b101:
                            //LHU
                            reg_living.write(in.rd,in.wb_value);
                            break;
                    }
                    break;
                case (unsigned int)0b0010011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            //ADDI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b001:
                            //SLLI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b010:
                            //SLTI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b011:
                            //SLTIU
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b100:
                            //XORI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b101:
                            //SRLI||SRAI
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //SRLI
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                                case 0b0100000:
                                    //SRAI
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                            }
                            break;
                        case 0b110:
                            //ORI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b111:
                            //ANDI
                            reg_living.write(in.rd,in.wb_value);
                            break;
                    }
                    break;
                case (unsigned int)0b0110011:
                    switch(in.funct3)
                    {
                        case 0b000:
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //ADD
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                                case 0b0100000:
                                    //SUB
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                            }
                            break;
                        case 0b001:
                            //SLL
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b010:
                            //SLT
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b011:
                            //SLTU
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b100:
                            //XOR
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b101:
                            switch(in.funct7)
                            {
                                case 0b0000000:
                                    //SRL
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                                case 0b0100000:
                                    //SRA
                                    reg_living.write(in.rd,in.wb_value);
                                    break;
                            }
                            break;
                        case 0b110:
                            //OR
                            reg_living.write(in.rd,in.wb_value);
                            break;
                        case 0b111:
                            //AND
                            reg_living.write(in.rd,in.wb_value);
                            break;
                    }
                    break;
            }
            if(waitClock_mirror.MEM<0)
                waitClock.WB=-1;
            //std::cerr<<"finish"<<std::endl;
        }
    }

    struct RUN_REG
    {
        unsigned int pc=0;
        if_id if_to_id;
        id_ex id_to_ex;
        ex_mem ex_to_mem;
        mem_wb mem_to_wb;

        unsigned int mirror_pc;
        if_id mirror_if_to_id;
        id_ex mirror_id_to_ex;
        ex_mem mirror_ex_to_mem;
        mem_wb mirror_mem_to_wb;

        bool mirror_if_run=false;
        bool mirror_id_run=false;
        bool mirror_ex_run=false;
        bool mirror_mem_run=false;

        bool if_run=false;
        bool id_run=false;
        bool ex_run=false;
        bool mem_run=false;

        bool run_with_out_if=false;
    }runReg;

    void load()
    {
        waitClock_mirror=waitClock;
        runReg.mirror_pc=runReg.pc;
        runReg.mirror_if_to_id=runReg.if_to_id;
        runReg.mirror_id_to_ex=runReg.id_to_ex;
        runReg.mirror_ex_to_mem=runReg.ex_to_mem;
        runReg.mirror_mem_to_wb=runReg.mem_to_wb;
        runReg.mirror_if_run=runReg.if_run;
        runReg.mirror_id_run=runReg.id_run;
        runReg.mirror_ex_run=runReg.ex_run;
        runReg.mirror_mem_run=runReg.mem_run;
        runReg.pc+=4;
    }

    void cal()
    {
        IF(runReg.mirror_pc,runReg.if_to_id);
        ID(runReg.mirror_if_to_id,runReg.id_to_ex);
        EX(runReg.mirror_id_to_ex,runReg.ex_to_mem);
        MEM(runReg.mirror_ex_to_mem,runReg.mem_to_wb);
        WB(runReg.mirror_mem_to_wb);
    }

public:
    mem mem_living;
    reg reg_living;
    void run()
    {
        //std::cerr<<"=====\ncmd read in"<<std::endl;
        reg_living.debug_print();
        while(true)
        {
            //std::cerr<<"=====\nclock "<<std::dec<<clock_cnt++<<std::endl;
            if(clock_cnt==53)
                int a=0;
            load();
            cal();
            //reg_living.debug_print();
            if(runReg.mem_to_wb.cmd==(unsigned int)0x0ff00513)
            {
                //final code running
                break;
            }
        }
        //std::cerr<<"run into final"<<std::endl;
        /*waitClock.IF=-1;
        waitClock.ID=-1;
        waitClock.EX=-1;
        waitClock.MEM=-1;
        while(waitClock.WB!=-1)
        {
            //std::cerr<<"=====\nfinal clock "<<std::dec<<clock_cnt++<<std::endl;
            load();
            cal();
            reg_living.debug_print();
        }*/
        std::cout<<(((unsigned int)reg_living.read(10)) & 255u)<<std::endl;
    }
};

#endif //RISCV_SIMULATOR_CORE_H
