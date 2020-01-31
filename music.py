import unittest
import re
"""
1.以01234567为基本四分音符，每个音符用英文逗号分开，如：1,2,3,4等，0代表休止符。
2.在音符前输入：“+-#!” 加号表示此音升高八度，减号则降八度，井号升半音，叹号降半音。
3.在基本音符后加斜杠“/”表示此音时值减一半，半成8分音符，双斜杠则变成16分音符。
4.可在以上基础上再加入延音线“-”（减号），一个减号表示延长一倍时值，和简谱里用法一样。
5.还可在以上基础上再加入附点音符“.”（英文的句号），作用与简谱一样（允许双附点）。

"""

"""
    stone is 0-7
    time is 1 2 4 8 F  
    octave and semitone is -1 0 1
    LOW8bit=INDEX
    HIGHT8BIT:
    8-11 is time
    12-13 is octave:   noram 00      octave:01      downtave:11
    14-15 is semitone  normal 00     upsemitone:01  downsemitone:11
"""
def create_stone(stone,time,octave,semitone):
    result=0
    result|=stone
    time=int(time)
    if time==16:
        time=0x0F
    result|=time<<8
    if octave==-1:
        octave=3
    result|=octave<<12
    if  semitone==-1:
        semitone=3
    result|=semitone<<14
    return result

test_str="""
3/,3/,3/,2/,3-,2/,3/,2/,2/,-6,-6/,-7/,1,2/,1/,-7.,-5/,-6---,3/,3/,3/,2/,3.,6/,5/,6/,5/,5/,2,2/,3/,4/,5/,4/,3.,2/,3---,6,-7/,-6/,5.,3/,5--,3/,5/,2,6/,5/,3.,2/,3---,2,5/,6.,1,6/,6/,6,6/,7/,+1,7/,6/,7.,5/,3---,6,7/,6/,5.,3/,5--,4/,5/,6,7/,6/,7,5.,6//,3---,2,5/,6.,1,6/,6.,6/,7/,+1,7/,6/,7.,5/,6----
"""

def get_stones(input_str):
    arr=input_str.split(",")
    result=[]
    for i in arr:
        num_result=re.search(r'(.*)(\d)(.*)',i)
        stone=int(num_result.group(2))
        before=num_result.group(1)
        behind=num_result.group(3)
        time=4
        half_count=behind.count("/")+1
        if(half_count==3):
            half_count=4
        double_count=behind.count("-")+1
        if(double_count>=3):
            double_count=4

        time*=half_count
        time/=double_count

        if before.find("+")!=-1:
            octave=1
        elif before.find("-")!=-1:
            octave=-1
        else:
            octave=0

        if i.find("#")!=-1:
            s=1
        elif i.find("!")!=-1:
            s=-1
        else:
            s=0
        stone=create_stone(stone,time,octave,s)
        result.append(stone)
    return result

for i in get_stones(test_str):
    print("%#x," %i,end="")


    
class Test(unittest.TestCase):
    def test_create_stone(self):
        stone=create_stone(5,4,1,0)
        self.assertEqual(stone,5125)
        stone=create_stone(5,16,1,0)
        self.assertEqual(stone,7941)

unittest.main()

