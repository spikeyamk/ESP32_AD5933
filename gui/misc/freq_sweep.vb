'-------------------------------------------------------------------------------------------------------
'Code developed using visual basic® 6.
'Datatype range
'Byte 0-255
'Double -1.797e308 to – 4.94e-324 and 4.94e-324 to 1.7976e308
'Integer -32,768 to 32767
'Long -2,147,483,648 to 2,147,483,647
'Variant‘...when storing numbers same range as double. When storing strings same range as string.
'-------------------------------------- Variable Declarations -----------------------------------------
Dim ReadbackStatusRegister As Long 'stores the contents of the status register.
Dim RealData As Double 'used to store the 16 bit 2s complement real data.
Dim RealDataUpper As Long 'used to store the upper byte of the real data.
Dim RealDataLower As Long 'used to store the lower byte of the real data.
Dim ImagineryData As Double 'used to store the 16 bit 2s complement real data.
Dim ImagineryDataLower As Long 'used to store the upper byte of the imaginary data.
Dim ImagineryDataUpper As Long 'used to store the lower byte of the imaginary data.
Dim Magnitude As Double 'used to store the sqrt (real^2+imaginary^2).
Dim Impedance As Double 'used to store the calculated impedance.
Dim MaxMagnitude As Double 'used to store the max impedance for the y axis plot.
Dim MinMagnitude As Double 'used to store the min impedance for the y axis plot.
Dim sweep_phase As Double 'used to temporarily store the phase of each sweep point.
Dim Frequency As Double 'used to temporarily store the current sweep frequency.
Dim Increment As Long 'used as a temporary counter
Dim i As Integer 'used as a temporary counter in (max/min) mag,phase loop
Dim xy As Variant 'used in the stripx profile
Dim varray As Variant
Dim Gainfactor As Double 'either a single mid point calibration or an array of calibration points
Dim TempStartFrequency As Double
Dim StartFrequencybyte0 As Long
Dim StartFrequencybyte2 As Long
Dim StartFrequencybyte1A As Long
Dim StartFrequencybyte1B As Long
Dim DDSRefClockFrequency As Double
Dim NumberIncrementsbyte0 As Long
Dim NumberIncrementsbyte1 As Long
Dim FrequencyIncrementbyt0 As Long
Dim FrequencyIncrementbyt1 As Long
Dim FrequencyIncrementbyt2 As Long
Dim SettlingTimebyte0 As Long
Dim SettlingTimebyte1 As Long
'-------------------------------------- I^2C read/write definitions-----------------------------------------
'used in the main sweep routine to read and write to AD5933.This is the vendor request routines in the firmware
Private Sub WritetToPart(RegisterAddress As Long, RegisterData As Long)
    PortWrite &HD, RegisterAddress, RegisterData
'parameters = device address register address register data
End Sub
Public Function PortWrite(DeviceAddress As Long, AddrPtr As Long, DataOut As Long) As Integer
    PortWrite = VendorRequest(VRSMBus, DeviceAddress, CLng(256 * DataOut + AddrPtr), VRWRITE, 0, 0)
End Function
Public Function PortRead(DeviceAddress As Long, AddrPtr As Long) As Integer
    PortRead = VendorRequest(VRSMBus, DeviceAddress, AddrPtr, VRREAD, 1, DataBuffer(0))
    PortRead = DataBuffer(0)
End Function
'------------------------------------- PHASE CONVERSION FUNCTION DEFINITION --------------------------------
'This function accepts the real and imaginary data(R, I) at each measurement sweep point and converts it to a degree
'-----------------------------------------------------------------------------------------------------------
Public Function phase_sweep(ByVal img As Double, ByVal real As Double) As Double
    Dim theta As Double
    Dim pi As Double
    pi = 3.141592654
    If ((real > 0) And (img > 0)) Then
        theta = Atn(img / real) ' theta = arctan (imaginary part/real part)
        phase2 = (theta * 180) / pi 'convert from radians to degrees
    ElseIf ((real > 0) And (img < 0)) Then
        theta = Atn(img / real) '4th quadrant theta = minus angle
        phase2 = ((theta * 180) / pi) + 360
    ElseIf ((real < 0) And (img < 0)) Then
        theta = -pi + Atn(img / real) '3rd quadrant theta img/real is positive
        phase2 = (theta * 180) / pi
    ElseIf ((real < 0) And (img > 0)) Then
        theta = pi + Atn(img / real) '2nd quadrant img/real is neg
        phase2 = (theta * 180) / pi
    End If
End Function
'-----------------------------------------------------------------------------------------------------------
Private Sub Sweep()
    ' the main sweep routine
    'This routine coordinates a frequency sweep using a mid point gain factor (see datasheet).
    'The gain factor at the mid-point is determined from the real and imaginary contents returned at this mid
    'point frequency and the calibration impedance.
    'The bits of the status register are polled to determine when valid data is available and when the sweep is
    'complete.
    '-----------------------------------------------------------------------------------------------------------
    IndexArray = 0 'initialize counter variable.
    Increment = NumberIncrements + 1 'number of increments in the sweep.
    Frequency = StartFrequency 'the sweep starts from here.
    '------------------------- PROGRAM 30K Hz to the START FREQUENCY register ---------------------------------
    DDSRefClockFrequency = 16000000.0 'Assuming a 16 MHz clock connected to MCLK
    StartFrequency = 30000.0 'frequency sweep starts at 30 KHz
    TempStartFrequency = (StartFrequency / (DDSRefClockFrequency / 4)) * 2 ^ 27 'dial up code for the DDS
    TempStartFrequency = Int(TempStartFrequency) '30 KHz = 0F5C28 hex
    StartFrequencybyte0 = 40 '40 DECIMAL = 28 HEX
    StartFrequencybyte1 = 92 '92 DECIMAL = 5C HEX
    StartFrequencybyte2 = 15 '15 DECIMAL = 0F HEX
    'Write in data to Start frequency register
    WritetToPart &H84, StartFrequencybyte0 '84 hex lsb
    WritetToPart &H83, StartFrequencybyte1 '83 hex
    WritetToPart &H82, StartFrequencybyte2 '82 hex
    '--------------------------------- PROGRAM the NUMBER OF INCREMENTS register ------------------------------
    'The sweep is going to have 150 points 150 DECIMAL = 96 hex
    'Write in data to Number Increments register
    WritetToPart &H89, 96 'lsb
    WritetToPart &H88, 0 'msb
    '--------------------------------- PROGRAM the FREQUENCY INCREMENT register ------------------------------
    'The sweep is going to have a frequency increment of 10Hz between successive points in the sweep
    DDSRefClockFrequency = 16000000.0 'Assuming a 16M Hz clock connected to MCLK
    FrequencyIncrements = 10 'frequency increment of 10Hz
    TempStartFrequency = (FrequencyIncrements / (DDSRefClockFrequency / 4)) * 2 ^ 27 'dial up code for the DDS
    TempStartFrequency = Int(TempStartFrequency) '10 Hz = 335 decimal = 00014F hex
    FrequencyIncrementbyt0 = 4.0F '335 decimal = 14f hex
    FrequencyIncrementbyt1 = 1
    FrequencyIncrementbyt2 = 0
    'Write in data to frequency increment register
    WritetToPart &H87, FrequencyIncrementbyt0 '87 hex lsb
    WritetToPart &H86, FrequencyIncrementbyt1 '86 hex
    WritetToPart &H85, FrequencyIncrementbyt2 '85 hex msb
    '--------------------------------- PROGRAM the SETTLING TIME CYCLES register ------------------------------
    'The DDS is going to output 15 cycles of the output excitation voltage before the ADC will start sampling
    'the response signal. The settling time cycle multiplier is set to x1
    SettlingTimebyte0 = 0F '15 cycles (decimal) = 0F hex
    SettlingTimebyte1 = 0 '00 = X1
    WritetToPart &H8B, SettlingTimebyte0
    WritetToPart &H8A, SettlingTimebyte1
'-------------------------------------- PLACE AD5933 IN STANDBYMODE ----------------------------------------
    'Standby mode command = B0 hex
    WritetToPart &H80, &HB0
'------------------------- Program the system clock and output excitation range and PGA setting-----------
    'Enable external Oscillator
    WritetToControlRegister2 &H81, &H8
'Set the output excitation range to be 2vp-p and the PGA setting to = x1
    WritetToControlRegister2 &H80, &H1
'-----------------------------------------------------------------------------------------------------------
    '------------- ------------ Initialize impedance under test with start frequency ---------------------------
    'Initialize Sensor with Start Frequency
    WritetToControlRegister &H80, &H10
    msDelay 2 'this is a user determined delay dependent upon the network under analysis (2ms delay)
    '-------------------------------------- Start the frequency sweep ------------------------------------------
    'Start Frequency Sweep
    WritetToControlRegister &H80, &H20
'Enter Frequency Sweep Loop
    ReadbackStatusRegister = PortRead(&HD, &H8F)
    ReadbackStatusRegister = ReadbackStatusRegister And &H4 ' mask off bit D2 (i.e. is the sweep complete)
    Do While ((ReadbackStatusRegister <> 4) And (Increment <> 0))
        'check to see if current sweep point complete
        ReadbackStatusRegister = PortRead(&HD, &H8F)
        ReadbackStatusRegister = ReadbackStatusRegister And &H2
        'mask off bit D1 (valid real and imaginary data available)
        '-------------------------------------------------------------------------
        If (ReadbackStatusRegister = 2) Then
            ' this sweep point has returned valid data so we can proceed with sweep
        Else
            Do
                'if valid data has not been returned then we need to pole stat reg until such time as valid data
                'has been returned
                'i.e. if point is not complete then Repeat sweep point and pole status reg until valid data returned
                WritetToControlRegister &H80, &H40 'repeat sweep point
                Do
                    ReadbackStatusRegister = PortRead(&HD, &H8F)
                    ReadbackStatusRegister = ReadbackStatusRegister And &H2
                    ' mask off bit D1- Wait until dft complete
                Loop While (ReadbackStatusRegister <> 2)
            Loop Until (ReadbackStatusRegister = 2)
        End If
        '-------------------------------------------------------------------------
        RealDataUpper = PortRead(&HD, &H94)
        RealDataLower = PortRead(&HD, &H95)
        RealData = RealDataLower + (RealDataUpper * 256)
        'The Real data is stored in a 16 bit 2's complement format.
        'In order to use this data it must be converted from 2's complement to decimal format
        If RealData <= &H7FFF Then ' h7fff 32767
            ' Positive
        Else
            ' Negative
            ' RealData = RealData And &H7FFF
            RealData = RealData - 65536
        End If
        ImagineryDataUpper = PortRead(&HD, &H96)
        ImagineryDataLower = PortRead(&HD, &H97)
        ImagineryData = ImagineryDataLower + (ImagineryDataUpper * 256)
        'The imaginary data is stored in a 16 bit 2's complement format.
        'In order to use this data it must be converted from 2's complement to decimal format
        If ImagineryData <= &H7FFF Then
            ' Positive Data.
        Else
            ' Negative
            ' ImagineryData = ImagineryData And &H7FFF
            ImagineryData = ImagineryData - 65536
        End If
        '-----------------Calculate the Impedance and Phase of the data at this frequency sweep point -----------
        Magnitude = ((RealData ^ 2) + (ImagineryData ^ 2)) ^ 0.5
        'the next section calculates the phase of the dft real and imaginary components
        'phase_sweep calculates the phase of the sweep data.
        sweep_phase = (phase_sweep(ImagineryData, RealData) - calibration_phase_mid_point)
        Gainfactor = xx 'this is determined at calibration. See gain factor section and Datasheet.
        Impedance = 1 / (Magnitude * Gainfactor)
        ' Write Data to each global array.
        MagnitudeArray(IndexArray) = Impedance
        PhaseArray(IndexArray) = sweep_phase
        ImagineryDataArray(IndexArray) = ImagineryData
        code(IndexArray) = Magnitude
        RealDataArray(IndexArray) = RealData
        Increment = Increment - 1 ' increment was set to number of increments of sweep at the start
        FrequencyPoints(IndexArray) = Frequency
        Frequency = Frequency + FrequencyIncrements ' holds the current value of the sweep freq
        IndexArray = IndexArray + 1
        '------------- Check to see if sweep complete ----------------------
        ReadbackStatusRegister = PortRead(&HD, &H8F)
        ReadbackStatusRegister = ReadbackStatusRegister And &H4 ' mask off bit D2
        'Increment to next frequency point Frequency
        WritetToControlRegister &H80, &H30
Loop
    '--------------------- END OF SWEEP: Place device into POWERDOWN mode------------------------------------
    'Enter Powerdown Mode,Set Bits D15,D13 in Control Register.
    WritetToPart &H80, &HA0
End Sub
'--------------------------------------------------------------------------------------------------------
'The programmed sweep is now complete and the impedance and phase data is available to read in the two
'arrays MagnitudeArray() = Impedance and PhaseArray() = phase.
sweepErrorMsg:
MsgBox "Error completing sweep check values"
End Sub
'The programmed sweep is now complete and the impedance and phase data is available to read in the two
'arrays MagnitudeArray() = Impedance and PhaseArray() = phase.