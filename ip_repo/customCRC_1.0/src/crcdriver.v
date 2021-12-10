module crcdriver(
  input wire clk,
  input wire nrst,
  input wire en,
  input wire [31:0] data,
  output wire [2:0] addr,
  output wire done,
  output wire [15:0] crc,
  input wire [7:0] seed,
  input wire [7:0] length,
  //output reg [7:0] counter
  output wire [7:0] counter,
  output reg [7:0] compcycles,
  output reg [7:0] randctr
  );

  wire crc_en, init;
  reg [7:0] crc_data;
  crc16 CRCCALC(
    .clk(clk),
    .nrst(nrst),
    .init(init),
    .en(crc_en),
    .data(crc_data),
    .seed(seed),
    .crc(crc)
    );

  reg [4:0] byte_ctr;

  reg [2:0] state;
  parameter S_IDLE = 0;
  parameter S_INIT = 1;
  parameter S_COMPUTE = 2;
  parameter S_DONE = 3;
  always@(posedge clk)
    if (!nrst)
      state <= S_IDLE;
    else
      case(state)
        S_IDLE:
          if (en)
            state <= S_INIT;
          else
            state <= S_IDLE;
        S_INIT:
          state <= S_COMPUTE;
        S_COMPUTE:
          if (byte_ctr == length[4:0])
            state <= S_DONE;
          else
            state <= S_COMPUTE;
        S_DONE:
          if (!en)
            state <= S_IDLE;
          else
            state <= S_DONE;
        default:
          state <= S_IDLE;
      endcase

  assign init = (state == S_INIT)? 1'b1 : 0;
  assign crc_en = (state == S_COMPUTE)? 1'b1 : 0;
  assign done = (state == S_DONE)? 1'b1 : 0;
  assign addr = byte_ctr[4:2];

  always@(*)
    case(byte_ctr[1:0])
      2'd0: crc_data <= data[31:24];
      2'd1: crc_data <= data[23:16];
      2'd2: crc_data <= data[15:8];
      2'd3: crc_data <= data[7:0];
    endcase

  always@(posedge clk)
    if (!nrst)
      byte_ctr <= 0;
    else
      /*if (state == S_COMPUTE)
        byte_ctr <= byte_ctr + 1;
      else
        byte_ctr <= 0;*/
      case(state)
        S_IDLE:
          byte_ctr <= 0;
        S_COMPUTE:
          byte_ctr <= byte_ctr + 1;
        default:
          byte_ctr <= byte_ctr;
      endcase
  assign counter = {3'd0,byte_ctr};
        
  /*always@(posedge clk)
    if (!nrst)
      counter <= 0;
    else
      case(state)
        S_IDLE:
          counter <= 0;
        S_COMPUTE:
          counter <= counter + 1;
        default:
          counter <= counter;
      endcase*/
      
  always@(posedge clk)
    if (!nrst)
      compcycles <= 0;
    else
      if (state == S_COMPUTE)
        compcycles <= compcycles + 1;
      else
        compcycles <= compcycles;
        
  always@(posedge clk)
    if (!nrst)
      randctr <= 0;
    else
      randctr <= randctr + 1;

endmodule
