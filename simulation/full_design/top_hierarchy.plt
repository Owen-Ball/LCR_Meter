[Transient Analysis]
{
   Npanes: 1
   {
      traces: 2 {268959747,0,"V(i_sns_pga)"} {524290,0,"V(i_sns)"}
      X: ('µ',0,0.00042,2e-05,0.00068)
      Y[0]: (' ',1,-2,0.4,2)
      Y[1]: ('µ',1,1e+308,8e-07,-1e+308)
      Volts: (' ',0,0,1,-2,0.4,2)
      Log: 0 0 0
   }
}
[AC Analysis]
{
   Npanes: 1
   {
      traces: 2 {524290,0,"V(i_sns_pga)"} {524291,0,"V(v_sns_pga)"}
      X: ('M',0,10,0,1e+07)
      Y[0]: (' ',0,1e-10,20,10)
      Y[1]: (' ',0,-490,70,210)
      Log: 1 2 0
      PltMag: 1
      PltPhi: 1 0
   }
}
