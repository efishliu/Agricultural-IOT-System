#include<reg52.h>
#include<intrins.h>
#include<FM1702.h>
#define uchar  unsigned char
#define uint   unsigned int
uchar code keya[]={0xff,0xff,0xff,0xff,0xff,0xff};
uchar code keyb[]={0xff,0xff,0xff,0xff,0xff,0xff};//卡的密匙
uchar code changekey[12]={0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f};//密匙变换存储区
uchar Fbuff[16];     //发送FIFO缓存
uchar Jbuff[16];     //接收FIFO缓存
uchar UID[7];      //卡型及卡号
uchar change[4];
sbit  cs=P2^0;
sbit  sck=P2^1;
sbit  mosi=P2^2;
sbit  miso=P2^3;
sbit  FM1702rst=P3^4;
sbit led9=P1^0;
sbit led8=P1^1; 
sbit led7=P1^2;
sbit led6=P1^3;
sbit led5=P1^4;
/*****************函数声明**************************/
//void  spi_write(uchar m);
uchar spi(uchar m);
void  fifo_clear();
void  resig_write(uchar reg,uchar da);
uchar resig_read(uchar reg);
uchar  fifo_read(uchar count,uchar *s);
void  fifo_write(uchar count,uchar *s);
uchar inti_1702();
void  card_halt();
void  card_scan();
uchar card_type();
uchar card_anticoll();
void  card_select();
void  card_authtication1(uchar m);
uchar card_authtication2();
uchar card_read(uchar m);
uchar card_write(uchar m);
uchar card_incre(uchar m,uchar x);
uchar card_dec(uchar m,uchar x);
uchar  loadkey();
void  delay(uchar m);
uchar  HL_card_active  (uchar picc_k);




uchar spi(uchar m)
{
        uchar i,temp=0;
        for(i=0;i<8;i++)
        {
                sck=0;
                if(m&0x80)
                mosi=1;
                else
                mosi=0;
                m<<=1;
                sck=1;
                temp<<=1;
                if(miso)
                temp|=0x01;

        }
        sck=0;
        mosi=0;
        return  temp;
}
void resig_write(uchar reg,uchar da)
{
        sck=0;
        reg<<=1;
        cs=0;
        reg=reg&0x7e;
        spi(reg);
        spi(da);
        cs=1;
}
uchar resig_read(uchar reg)
{
        uchar temp;
        sck=0;
        _nop_();
        _nop_();
        cs=0;
        reg<<=1;
        reg|=0x80;
        spi(reg);
        temp=spi(0x00);
        cs=1;
        return temp; 
        
}
void  fifo_clear()
{
        uchar temp;
        temp=resig_read(Control);
        temp=temp|0x01;
        resig_write(Control,temp);
        while(resig_read(FIFOLength))        ;
}
uchar fifo_read(uchar count,uchar *s)
{
        uchar i,temp;
        temp=resig_read(FIFOLength);
        if(temp<count)
        return  0;
        else
        {
                for(i=0;i<count;i++)
                {                                        
                        temp=resig_read(FIFODaTa);
                        *(s+i)=temp;
                }                
        }

}
void fifo_write(uchar count,uchar *s)
{
        uchar i,temp;
    fifo_clear();
        for(i=0;i<count;i++)
        {
                temp=*(s+i);
                resig_write(FIFODaTa,temp);
        }
}
uchar  inti_1702()
{
        uchar temp;
        FM1702rst=1;
    delay(0x0a);                        
        FM1702rst=0;                                 
        delay(0x0f);
        temp=resig_read(Command);   
        //////
        if(temp==0)
        resig_write(0x00,0x80);   
        
        temp=resig_read(Command);             
        while(temp);
        
        if(temp==0x00)           
        return 1;          
        else
        return 0;         
}
void delay(uchar m)    //单次定时为10ms
{
        TMOD=0x01;
        while(m--)
                {        
                         TH0=0xdc;
                         TL0=0x00 ;
                        TR0=1;
                        while(!TF0);
                        TF0=0;
                        TR0=0;
                }
           
}
void  card_scan()
{
        uchar  temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(BitFraming,0x07);
        resig_write(ChannelRedundancy,0x03);
        temp=resig_read(Control);
        temp=temp&0x7f;
        resig_write(Control,temp);
        Fbuff[0]=0x52;                          
        fifo_write(1,Fbuff);                            
        resig_write(Command,0x1e);                                                                     
        fifo_clear();                                    
}
uchar  card_type()
{
        uchar temp1,temp2;
//        fifo_read(2,UID);
//        temp1=UID[0];
//        temp2=UID[1];
        temp1=resig_read(FIFOLength);   
//        if((temp1==0x04)||(temp1==0x03)||(temp1==0x05)||(temp1==0x53)&&(temp2==0))
//        {        
                if(temp1==0x02)
                return 0x01;
        //        fifo_read(2,UID);
//        }
        else
        return 0;
}
uchar card_anticoll()                                                        //防冲突函数
{
        uchar temp,i;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x03);
        Fbuff[0]=0x93;
        Fbuff[1]=0x20;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
        delay(1);
        temp=resig_read(FIFOLength);
        if(temp==0x05)        
        {
                temp=0;
                fifo_read(5,UID);
                for(i=0;i<5;i++)
                {
                        temp=temp^UID;        
                }
                if(temp==0)
                return 1;
                else
                return 0;
        }
        else
        return 0;
}
void   card_select()
{
        uchar i;
        resig_write(Control,0x01)  ;
        Fbuff[0]=0x93;
        Fbuff[0]=0x70;
        for(i=0;i<5;i++)
        {
                Fbuff[i+2]=UID;
        }
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x0f);
        fifo_write(7,Fbuff);
        resig_write(Command,0x1e);
}
void    card_authtication1(uchar m)
{
        resig_write(Control,0x01)  ;
        Fbuff[0]=0x60;
        Fbuff[1]=m;
        fifo_write(6,Fbuff);
        resig_write(Command,0x0c);
}
uchar    card_authtication2()
{
        uchar temp;
        resig_write(Control,0x01)  ;
        resig_write(Command,0x14);
    temp=resig_read(Control);
        temp=temp&0x08;
        if(temp!=0x08)
        return 1;
        else
        return 0;
}
uchar card_read(uchar m)
{
        uchar temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x0f);
        resig_write(Control,0x01);
        Fbuff[0]=0x30;
        Fbuff[1]=m;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
        temp=resig_read(FIFOLength);
        if(temp==16)
        {
                return 1;
                fifo_read(16,Jbuff);
        }
        else
        return 0;        
}
uchar  card_write(uchar m)
{
        uchar temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x0f);
        resig_write(Control,0x01);
        Fbuff[0]=0xA0;
        Fbuff[1]=m;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
        temp=resig_read(FIFOLength);
        if(temp==1)
        {
                return 1;
                resig_write(Control,0x01);
                fifo_write(16,Fbuff);
        }
        else
        return 0;        
}
uchar  card_incre(uchar m,uchar x)
{
        uchar temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x07);
        resig_write(Control,0x01);
        Fbuff[0]=0xC1;
        Fbuff[1]=m;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
        temp=resig_read(FIFOLength);
        if(temp==0x01)
        {
                
                resig_write(Control,0x01); 
                change[0]=x;
                fifo_write(4,change);
            resig_write(Command,0x1a);
                return 1;
        }
        return   0;
}
uchar  card_dec(uchar m,uchar x)
{
        uchar temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x07);
        resig_write(Control,0x01);
        Fbuff[0]=0xC0;
        Fbuff[1]=m;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
        temp=resig_read(FIFOLength);
        if(temp==0x01)
        {
                return 1;
                resig_write(Control,0x01);
                change[0]=x;
                fifo_write(4,change);
            resig_write(Command,0x1a);
        }
        return  0;
}
uchar loadkey()
{
        uchar temp;
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x0f);
        resig_write(Control,0x01);
        fifo_write(12,changekey);
        resig_write(Command,0x19);
        temp=resig_read(ErrorFlag);
        temp=temp&0x40;
        if(temp==0x40)
        return 0;
        else
        return 0x01;
}
void  card_halt()
{
        resig_write(CRCResultLSB,0x63);
        resig_write(CWConductance,0x3f);
        resig_write(ChannelRedundancy,0x03);
        resig_write(Control,0x01);
        Fbuff[0]=0x50;
        Fbuff[0]=0x00;
        fifo_write(2,Fbuff);
        resig_write(Command,0x1e);
}
uchar  HL_card_active  (uchar picc_k)
{
        uchar temp;
    card_halt();
        temp=card_type();
        if(temp)
        {
                temp=card_anticoll();
                if(temp)
                {
                    card_select();
                    card_authtication1(picc_k);
                        if(temp)
                        {
                                temp=card_authtication2();
                                if(temp)
                                {
                                        temp=loadkey();
                                        if(temp)
                                        {
                                        return 1          ;
                                        }
                                }        
                        }        
                }
        }

return 0;
}
void main()
{
        uchar temp;

        temp=inti_1702();
        if(temp==0x01)
        led9=0;         
        card_scan(); 
        temp=resig_read(PrimaryStatus);
        temp=temp&0xf0;
        if(temp==0x00)
        led8=0;          
        while(1)
        {        card_scan();        
                temp=card_type();        
                if(temp==0x01)
                {
                          led8=~led8;
                          delay(0x0f);
                }         
        //        card_anticoll()        ;              
        }
}

