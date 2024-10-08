# LTR11 Recording MATLAB Parser

This directory contains a MATLAB script which parses data from a BGT60LTR11 device.
The data can be recorded via Infineon's Radar Fusion GUI.
The script additionally runs an algorithm using the parsed data, calculates, and
displays the approaching or departing velocity of a detected object in a MATLAB plot.
This can be used as an example to read recording data, and to perform customized processing on it.

## Requirements

- MATLAB installation (tested with 2019b)
- Data from LTR11 recorded via Infineon's Radar Fusion GUI

## Content

The Content of this directory is the following
- recording_parser_script_BGT60LTR11.m : the BGT60LTR11 data parser script
- src: directory containing the parsing functions written in MATLAB.

## Usage

The usage steps are as follows
- Open an instance of MATLAB
- Switch the MATLAB working directory to the directory containing the parser script
- Run the script >> recording_parser_script_BGT60LTR11
- The user will be prompted for the recording file name (*.raw.bin)
- The script will parse the data, run and algorithm, and display the target velocity result as a MATLAB plot.
