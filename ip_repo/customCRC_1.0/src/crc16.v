module crc16(
//module crc16 #(parameter seed = 16'h555a)
//(
  input wire [7:0] data,
  input wire clk,
  input wire nrst,
  input wire init,
  input wire en,
  input wire [7:0] seed,
  output reg [15:0] crc
);

always @(posedge clk)
  if(!nrst)
    crc <= seed;
  else
    if (init)
      crc <= seed;
    else 
      if (en) begin
        crc[0] <= data[4] ^ data[0] ^ crc[8] ^ crc[12];
        crc[1] <= data[5] ^ data[1] ^ crc[9] ^ crc[13];
        crc[2] <= data[6] ^ data[2] ^ crc[10] ^ crc[14];
        crc[3] <= data[7] ^ data[3] ^ crc[11] ^ crc[15];
        crc[4] <= data[4] ^ crc[12];
        crc[5] <= data[5] ^ data[4] ^ data[0] ^ crc[8] ^ crc[12] ^ crc[13];
        crc[6] <= data[6] ^ data[5] ^ data[1] ^ crc[9] ^ crc[13] ^ crc[14];
        crc[7] <= data[7] ^ data[6] ^ data[2] ^ crc[10] ^ crc[14] ^ crc[15];
        crc[8] <= data[7] ^ data[3] ^ crc[0] ^ crc[11] ^ crc[15];
        crc[9] <= data[4] ^ crc[1] ^ crc[12];
        crc[10] <= data[5] ^ crc[2] ^ crc[13];
        crc[11] <= data[6] ^ crc[3] ^ crc[14];
        crc[12] <= data[7] ^ data[4] ^ data[0] ^ crc[4] ^ crc[8] ^ crc[12] ^ crc[15];
        crc[13] <= data[5] ^ data[1] ^ crc[5] ^ crc[9] ^ crc[13];
        crc[14] <= data[6] ^ data[2] ^ crc[6] ^ crc[10] ^ crc[14];
        crc[15] <= data[7] ^ data[3] ^ crc[7] ^ crc[11] ^ crc[15];
      end
      else
        crc <= crc;

endmodule

