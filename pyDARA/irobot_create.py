import serial
import time

class Roomba(object):
    
    def __init__(self, port_name, is_create_two=False):
        if(is_create_two):
            # Create 2 uses 115200
            self.roomba = serial.Serial(port_name,115200)
        else:
            # Create 1 uses 57600
            self.roomba = serial.Serial(port_name,57600)
        self.set_mode_passive()  
        self.set_mode_safe()
        
    def close(self):
        # Graceful sleep (allows for charging)
        self.set_mode_passive()
        time.sleep(1)
        self.set_mode_stop()      
        
    def _signed_word_to_bytes(self,value):
        # The roomba uses 16 bit signed words, MSB first
        if value<0:
            value = value + 65536
        return bytes([(value>>8)&255,value&255])
            
    ##### Mode Commands
    
    def set_mode_reset(self):
        # 07
        self.roomba.write(b'\x07')
        
    def set_mode_passive(self):
        # 128
        self.roomba.write(b'\x80')
        time.sleep(1)
        
    def set_mode_stop(self):
        # 173
        self.roomba.write(b'\xAD')
        
    # 81 129 - TODO SetBaud
    # 82 130 - TODO Control
    
    def set_mode_safe(self):
        # 131
        self.roomba.write(b'\x83')
        time.sleep(1)
    
    def set_mode_full(self):
        # 132
        self.roomba.write(b'\x84')
        time.sleep(1)
    
    # 85 133 Power (same thing as 83 passive)
    
    ##### Cleaning Algorithms
    
    def set_clean_spot(self):
        # 134
        self.roomba.write(b'\x86')
    
    def set_clean_clean(self):
        # 135
        self.roomba.write(b'\x87')
    
    def set_clean_max_clean(self):
        # 136
        self.roomba.write(b'\x88')
        
    ##### Drive
    
    def set_drive(self,velocity_mms,radius_mm):
        # 137
        # Special radius cases (handled in other functions)
        # straight = 32767 (forward) or -32768 (backwards)
        # spin CW = radius -1
        # spin CCW = radius 1
        cmd = b'\x89'+self._signed_word_to_bytes(velocity_mms)+self._signed_word_to_bytes(radius_mm)
        #print(cmd)
        #self.roomba.write(cmd)
        #cmd=b'\x89'
        #self.roomba.write(cmd)
        #cmd=b'\x00'
        #self.roomba.write(cmd)
        #cmd=b'\x60'
        #self.roomba.write(cmd)
        #cmd=b'\x00'
        #self.roomba.write(cmd)
        #cmd=b'\x00'
        #self.roomba.write(cmd)
        
    def set_drive_stop(self):
        cmd=b'\x89'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
    
    def set_drive_straight(self):
        cmd=b'\x89'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\xC8'
        self.roomba.write(cmd)
        cmd=b'\x80'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        
    def set_drive_spin_cw(self):
        cmd=b'\x89'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\xC8'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        cmd=b'\x00'
        self.roomba.write(cmd)
        
    def set_drive_spin_ccw(self,velocity_mms):
        self.set_drive(velocity_mms,1)
        
    def set_drive_direct(self,left_mms,right_mms):  
        # 145      
        # mm/s 
        # The manual says range is -500 to +500, but my roomba goes higher
        cmd = b'\x91'+ self._signed_word_to_bytes(right_mms)+self._signed_word_to_bytes(left_mms)
        print(str(cmd))
        self.roomba.write(cmd)
    
    def set_drive_pwm(self,left_pwm,right_pwm):
        # 146
        # pwm 
        # The manual says range is -255 to +255, but my roomba goes higher
        cmd = b'\x92'+ self._signed_word_to_bytes(right_pwm)+self._signed_word_to_bytes(left_pwm)
        print(str(cmd))
        self.roomba.write(cmd)
        
    def set_seek_dock(self):
        # 143
        cmd = b'\x8F'
        self.roomba.write(cmd)
        
    ##### Cleaning Motors
    
    def set_cleaning_motors(self,main_brush_direction,side_brush_cw,main_brush_on,vacuum_on,side_brush_on):
        # 8A 138
        # TODO
        pass
    
    def set_cleaning_motors_pwm(self,main_brush_pwm,side_brush_pwm,vacuum_pwm):
        # 90 144
        # TODO
        pass
        
    ##### LED Control
    
    def set_leds(self,check,dock,spot,debris,power_color,power_intensity):
        # 139
        # individual LEDs are on/off
        # power color and intensity are 0-255
        ind = 0
        if check:
            ind = ind | 8
        if dock:
            ind = ind | 4
        if spot:
            ind = ind | 2
        if debris:
            ind = ind | 1        
        cmd = bytes([0x8B,ind,power_color,power_intensity])        
        self.roomba.write(cmd)
        
    ##### Music Control
    
    def set_song(self,number,notes):
        # 140
        # number: 0,1,2, or 3
        # notes are pairs of midiNumber/duration
        #   midiNumber: 31 through 127 (outside this range is a silence)
        #   duration: 1/64ths of a second
        #
        # The documentation says song number are 0-4, which would be 5 total songs.
        # My experimentation shows only 0-3 are valid.
        #
        # Experimentation shows that each song can be at most 16 notes. If you pass in more
        # than 16 then the notes spill into the next song. This allows you to use song 0
        # as one big 16*4 note song.
        cmd = b'\x8C' + bytes([number,len(notes)])
        for note in notes:
            cmd = cmd + bytes([note[0],note[1]])
        self.roomba.write(cmd)          
    
    def play_song(self,number):
        # 141
        # number: 0,1,2, or 3
        cmd = b'\x8D' + bytes([number])
        self.roomba.write(cmd)    
    
    ##### Sensor Control
    
    def get_sensor_packet(self, sensor_object):
        # 142
        cmd = b'\x8E' + bytes([sensor_object.ID])
        self.roomba.write(cmd)
        data = self.roomba.read(sensor_object.SIZE)
        sensor_object.decode(data,0)        
    
    def get_sensor_multi_packets(self, sensor_objects):
        # 149 x95
        pass
    
    # 94 TODO 148 Stream
    # 96 TODO 150 Pause/Resume Stream
    
if __name__ == '__main__':
    
    #import sensor_packets
    import sensor_groups
    
    roomba = Roomba('/dev/ttyUSB0')
    #roomba = Roomba('COM4')
    
    roomba.set_mode_full()    
    
    for x in range(8):      
        sens = sensor_groups.CliffRight()
        roomba.get_sensor_packet(sens)
        print(sens)   
        time.sleep(1) 
    
    '''  
    roomba.set_drive_spin_ccw(100)
    time.sleep(2)
    roomba.set_drive_stop()
    
    sens = sensor_packets.Angle()
    roomba.get_sensor_packet(sens)
    print(sens)
    
    roomba.set_drive_spin_cw(150)
    time.sleep(2)
    roomba.set_drive_stop()
    
    sens = sensor_packets.Angle()
    roomba.get_sensor_packet(sens)
    print(sens)    
            
    time.sleep(2)
    '''
    
    roomba.set_mode_passive()
    time.sleep(1)
    roomba.set_mode_stop()
        
