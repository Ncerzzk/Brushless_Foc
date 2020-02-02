import serial

text=''
with open("voice.txt","r") as f:
    text=f.read()

str_arr=text.split(',')
result=[]
for i in str_arr:
    if(i==''):
        continue
    result.append(int(i))


def serial_opera(arr):
    com = "COM13"
    baudrate = 115200

    ser = serial.Serial(com, baudrate, 8)
    if(ser.isOpen()):
        print("open!")
    lenght=len(arr)
    index = 0
    while True:
        temp=result[index:index+1000]
        ser.write(bytes(temp))
        index+=1000
        if(index>=lenght-1000):
            index=0
                
                # if(index%1000==0):
                    # break


serial_opera(result)
