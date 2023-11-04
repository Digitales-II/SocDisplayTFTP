module ram #(parameter IMAGEN = "imagen1.txt")(
    // Clk
    input wire i_clk,
    // Lectura
    input wire rd,
    input [11:0] addrRead,
    output reg [53:0] o_valor,
    // Escritura
    input wire wr,
    input [11:0] addrWrite,
    input [53:0] dataLine
);
    reg [53:0] ext_mem [3200:0];

    initial begin
        $readmemb(IMAGEN,ext_mem);
    end

always @(posedge i_clk) begin
    // Lectura
	if (rd)	begin
	    o_valor <= ext_mem[addrRead];
    end
    // Escritura
    if (wr) begin
        ext_mem[addrWrite] <= dataLine;
        //$writememb(IMAGEN,ext_mem); // Para simulación se usa este.
    end
end
    
endmodule