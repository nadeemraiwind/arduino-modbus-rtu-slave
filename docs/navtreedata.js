/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Arduino Modbus RTU Slave", "index.html", [
    [ "Modbus RTU Arduino Library", "index.html", "index" ],
    [ "Known Issues and Limitations", "md__k_n_o_w_n___i_s_s_u_e_s.html", [
      [ "Recent Fixes (v1.1.0)", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md56", [
        [ "✅ <b>ADDED: Optional Static Register Pool Mode</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md57", null ],
        [ "✅ <b>FIXED: Broken Example Code</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md58", null ],
        [ "✅ <b>FIXED: CRC Table Linker Errors</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md59", null ],
        [ "✅ <b>IMPROVED: Example Documentation</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md60", null ],
        [ "✅ <b>FIXED: 8-Byte Read Assumption in run()</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md61", null ],
        [ "✅ <b>IMPROVED: Microsecond RTU Frame Timing</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md62", null ],
        [ "✅ <b>ADDED: FC15/FC16 Multi-Write Support</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md63", null ],
        [ "✅ <b>ADDED: Stream Transport Support</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md64", null ],
        [ "✅ <b>IMPROVED: get() Not-Found Signaling</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md65", null ]
      ] ],
      [ "Current Limitations", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md66", [
        [ "⚠️ <b>Fixed-Length Request Validation for FC01-FC06</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md67", null ],
        [ "⚠️ <b>Sentinel Collision Risk in get()</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md68", null ],
        [ "⚠️ <b>Linked List Lookup Cost (Default Mode)</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md69", null ],
        [ "⚠️ <b>Frame Completion Depends on RTU Silence Window</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md70", null ],
        [ "⚠️ <b>Baud Reconfiguration Depends on HardwareSerial</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md71", null ],
        [ "ℹ️ <b>No Modbus ASCII Support</b>", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md72", null ]
      ] ],
      [ "Best Practices", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md73", [
        [ "Register Management", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md74", null ],
        [ "Serial Communication", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md75", null ],
        [ "Memory Management", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md76", null ]
      ] ],
      [ "Supported Function Codes", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md77", null ],
      [ "Testing Recommendations", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md78", null ],
      [ "Found a Bug or Issue?", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md80", null ],
      [ "Acknowledgments", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md82", null ],
      [ "Future Enhancements (Suggestions)", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md84", [
        [ "Elite Level (Major Features)", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md85", null ],
        [ "Performance &amp; API Improvements", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md86", null ]
      ] ],
      [ "Version History", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md87", [
        [ "v1.1.0 (Current)", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md88", null ],
        [ "v0.1.x (Previous)", "md__k_n_o_w_n___i_s_s_u_e_s.html#autotoc_md89", null ]
      ] ]
    ] ],
    [ "System Architecture and Code Flow", "architecture.html", [
      [ "Request-Response Sequence Diagram", "architecture.html#sequence_diagram", null ],
      [ "run() State Machine Flowchart", "architecture.html#state_machine", null ],
      [ "processFrame() Validation Chain", "architecture.html#validation_chain", [
        [ "Check 1: Frame Length", "architecture.html#autotoc_md93", null ],
        [ "Check 2: CRC-16 Integrity", "architecture.html#autotoc_md94", null ],
        [ "Check 3: Slave ID Matching", "architecture.html#autotoc_md95", null ],
        [ "Check 4: Function Code Support", "architecture.html#autotoc_md96", null ],
        [ "Check 5: Data Validation", "architecture.html#autotoc_md97", null ]
      ] ],
      [ "Security and Robustness Features", "architecture.html#security", [
        [ "Frame Validation", "architecture.html#autotoc_md99", null ],
        [ "Buffer Safety", "architecture.html#autotoc_md100", null ],
        [ "Timing Robustness", "architecture.html#autotoc_md101", null ],
        [ "Atomic Protection", "architecture.html#autotoc_md102", null ]
      ] ],
      [ "Debugging Guide", "architecture.html#debugging", null ],
      [ "Performance Characteristics", "architecture.html#performance", [
        [ "Execution Time (Arduino Mega 16MHz)", "architecture.html#autotoc_md105", null ],
        [ "Memory Footprint", "architecture.html#autotoc_md106", null ],
        [ "Communication Limits", "architecture.html#autotoc_md107", null ]
      ] ],
      [ "Author &amp; Credits", "architecture.html#credits", [
        [ "Lead Developer", "architecture.html#autotoc_md110", null ],
        [ "Development Tools", "architecture.html#autotoc_md111", null ],
        [ "Special Acknowledgment", "architecture.html#autotoc_md112", null ],
        [ "Contributing", "architecture.html#autotoc_md113", null ]
      ] ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"modbus_8h_source.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';