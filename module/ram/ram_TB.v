`timescale 1 ns / 100 ps
`include "ram.v"

module ram_TB;

    parameter tck           = 20;       // clock period in ns
    parameter clk_freq      = 1000000000 / tck; // Frequenzy in HZ

    reg i_clk;
    reg request;
    wire [15:0] o_valor = 0; // Salidas del testbench deben ser un wire.

    ram uut (
        .i_clk      (i_clk),
        .request    (request),
        .o_valor    (o_valor)
    );

    initial begin
        $dumpfile("ram_TB.vcd");
        $dumpvars(-1,ram_TB);


        #(tck*15) // Tiempo máximo de la simulación
        $display("Simulación completada");
        $finish; // Sin el finish no acaba
    end
    
    /* Clocking device */
    initial begin
        i_clk <= 0; 
        request <= 0;
    end
    
    always #(tck/2) i_clk <= ~i_clk;   
    always #(2*tck/3) request <= ~request;   
    
endmodule