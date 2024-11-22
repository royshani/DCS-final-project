import PySimpleGUI as sg
import serial as ser
import sys, glob
import time

from pip._internal.utils.filesystem import file_size
from serial.tools import list_ports
from tkinter import *
import mouse
import os
from os import path
import threading
import binascii
import pyautogui

global painter_state
painter_state = 0  # Initialize painter_state to a default value


class Paint(object):
    # Default pen size and color for the painting tool
    DEFAULT_PEN_SIZE = 5.0
    DEFAULT_COLOR = 'black'
    ERASER_SIZE = 20.0  # Increased eraser size

    def __init__(self):
        # Initialize the main window for the painting application
        self.root = Tk()

        # Create and place the 'Back' button on the grid, used to close the painting application
        self.back_button = Button(self.root, text='Back', command=self.close_painter)
        self.back_button.grid(row=0, column=0)

        # Create and place a label to show the current painter state
        self.state_label = Label(self.root, text=self.get_state_text(), font=('Arial', 12))
        self.state_label.grid(row=0, column=1)

        # Create and place the 'Clear All' button on the grid to clear the canvas
        self.clear_button = Button(self.root, text='Clear All', command=self.clear_canvas)
        self.clear_button.grid(row=0, column=2)

        # Create and place a canvas on the grid, where the drawing takes place
        self.c = Canvas(self.root, bg='white', width=1000, height=600)
        self.c.grid(row=1, columnspan=3)

        # Set up the canvas and initialize the application
        self.setup()
        # Start the main loop of the Tkinter application
        self.root.mainloop()

    def setup(self):
        # Initialize variables to track the previous mouse coordinates
        self.old_x = None
        self.old_y = None
        # Set the initial line width
        self.line_width = self.DEFAULT_PEN_SIZE
        # Set the initial pen color
        self.color = self.DEFAULT_COLOR
        # Initialize the eraser mode as off
        self.eraser_on = False
        # Bind the paint function to mouse motion events on the canvas
        self.c.bind('<Motion>', self.paint)
        # Bind the reset function to mouse button release events on the canvas
        self.c.bind('<ButtonRelease-1>', self.reset)

    def get_state_text(self):
        global painter_state
        # Return a text representation of the current painter state
        if painter_state == 0:
            return 'State: Pen'
        elif painter_state == 1:
            return 'State: Eraser'
        elif painter_state == 2:
            return 'State: Neutral'
        else:
            return 'State: Unknown'

    def update_state_label(self):
        global painter_state
        # Update the state label text based on the current painter state
        self.state_label.config(text=self.get_state_text())

    def paint(self, event):
        global painter_state

        # Update the state label dynamically
        self.update_state_label()

        # Check the current painter_state and perform actions based on the painter_state
        if painter_state == 0 and self.old_x and self.old_y:  # paint mode
            # Draw a line on the canvas from the previous mouse position to the current one
            self.c.create_line(self.old_x, self.old_y, event.x, event.y,
                               width=self.line_width, fill=self.color,
                               capstyle=ROUND, smooth=TRUE, splinesteps=36)
        elif painter_state == 1 and self.old_x and self.old_y:  # erase mode
            # Erase by drawing a white line over the existing drawing
            self.c.create_line(self.old_x, self.old_y, event.x, event.y,
                               width=self.ERASER_SIZE, fill='white',  # Use larger width for eraser
                               capstyle=ROUND, smooth=TRUE, splinesteps=36)
        elif painter_state == 2:  # neutral painter_state (no action)
            pass

        # Update the old coordinates to the current ones for the next movement
        self.old_x = event.x
        self.old_y = event.y

    def reset(self, event):
        # Reset the old coordinates when the mouse button is released
        self.old_x, self.old_y = None, None

    def clear_canvas(self):
        # Clear the entire canvas
        self.c.delete("all")

    def close_painter(self):
        global PaintActive
        # Deactivate the painter mode and close the application
        PaintActive = 0
        self.root.destroy()

def start_painter(window):
    Paint()


'''
##########################################################################
                   Graphical User Interface
##########################################################################
'''
# Constants
THEMES = {'main': 'DarkAmber'}
STATES = {
    'manual': 'm', 'auto_rotate': 'A', 'stop': 'M', 'joystick': 'J',
    'painter': 'P', 'calibration': 'C', 'script': 's'
}
FILE_INDICES = {'f': 0, 'g': 1, 'h': 2}
EXECUTE_INDICES = {'F': 0, 'G': 1, 'H': 2}
global file_path
file_path = ''
global file_name        # initialize file name
file_name = ''          # initialize file name
global file_name_to_execute
file_name_to_execute = None
global execute_index
execute_index = None
global upload_index
upload_index = 0

def GUI():
    sg.theme('Material2')
  # Change to your preferred theme
    global file_name
    global file_name_to_execute
    global execute_index
    global upload_index
    # Define a new layout for the main menu
    layout_main = [
        [sg.Text("Roy & Yanai Final Project", size=(50, 2), justification='center', font='Cardamon 30')],
        [sg.Text("Main Menu", size=(50, 1), justification='center', font='Cardamon 25')],
        [sg.Button("1) Manual control of motor-based machine", key='_ManualStepper_', size=(40, 4), font='Cardamon 15'),
         sg.Button("2) Joystick based PC painter", key='_Painter_', size=(40, 4), font='Cardamon 15')],
        [sg.Button("3) Stepper Motor Calibration", key='_Calib_', size=(40, 4), font='Cardamon 15'),
         sg.Button("4) Script Mode", key='_Script_', size=(40, 4), font='Cardamon 15')]
    ]

    layout_manualstepper = [[sg.Text("Manual control of motor-based machine", size=(50, 2), justification='center', font='Cardamon 20')],
        [sg.Text("please rotate the pointer to 0 deg:", size=(75, 1),font='Cardamon 20')],
        [sg.Frame("",layout=[[sg.Button("Rotate Stepper Motor", key='_Rotation_', size=(75, 2), font='Cardamon 15')],
        [sg.Button("Stop Rotation", key='_Stop_', size=(75, 2), font='Cardamon 15')],
        [sg.Button("Switch to Joystick Control", key='_JoyStickCrtl_', size=(75, 2), font='Cardamon 15')]],
        title_color='blue',relief=sg.RELIEF_SUNKEN)],
        [sg.Text("", size=(50, 2))],  # Spacer
        [sg.Button("Back", key='_BackMenu_', size=(10, 2), font='Cardamon 15', pad=(0, 20))]]

    layout_painter = [
        [sg.Text("Joystick based PC painter", size=(35, 2), justification='center', font='Cardamon 15')],
        [sg.Canvas(size=(1000, 1000), background_color='red', key='canvas')],
        [sg.Button("Back", key='_BackMenu_', size=(5, 1), font='Cardamon 15', pad=(300, 180))]
    ]
#  Press again when it reaches the black line.
    layout_calib = [
        [sg.Text("Stepper Motor Calibration", size=(35, 2), justification='center', font='Cardamon 30')],
        [sg.Text("To start calibration process, press the joystick button. ", justification='center', font='Cardamon 20'),
         sg.Text(" ", size=(35, 2), key="first_line", justification='center', font='Cardamon 20')],
        [sg.Text("Press again when it reaches the black line.", justification='center',
                 font='Cardamon 20'),
         sg.Text(" ", size=(35, 2), key="sec_line", justification='center', font='Cardamon 20')],
        [sg.Text("Number of steps: ", justification='center', font='Cardamon 20'),
         sg.Text(" ", size=(35, 2), key="Counter", justification='center', font='Cardamon 20')],
        [sg.Text("Phi: ", justification='center', font='Cardamon 20'),
         sg.Text(" ", size=(35, 2), key="Phi", justification='center', font='Cardamon 20')],
        [sg.Button("Back", key='_BackMenu_', size=(5, 1), font='Cardamon 20')]
    ]

    file_viewer = [[sg.Text("Choose folder", size=(25, 2),  font='Cardamon 17')],
                   [sg.Text("Folder"),sg.In(size=(25, 1), enable_events=True, key='_Folder_'), sg.FolderBrowse()],
                   [sg.Listbox(values=[], enable_events=True, size=(40, 15), key="_FileList_")],
                   [sg.Button('Back', key='_BackMenu_', size=(7, 1), font='Cardamon 15')],
                    [sg.Button('Choose', key='_upload_', size=(7, 1), font='Cardamon 15')],
                    [sg.Text(' ', key='_Acknowledge_', size=(35, 1), font='Cardamon 10')]]

    file_description = [
        [sg.Text("Content", size=(15, 1), justification='center', font='Cardamon 25'), sg.Text("Files to execute", size=(15, 1), justification='center', font='Cardamon 25')],
        [sg.Text(size=(20, 1), key="_FileHeader_", font='Cardamon 11')],
        [sg.Multiline(size=(40, 16), key="_FileContent_"),
         sg.Listbox(values=[], enable_events=True, size=(40, 15), key="_ExecutedList_")],
        [sg.Button('Choose File to Execute', key='_Execute_', size=(40, 1), font='Cardamon 15')]
        #,[sg.Button('Delete', key='_Clear_', size=(40, 1), font='Cardamon 15')]
    ]

    layout_calc = [
        [sg.Text(" ", key='_FileName_', size=(35, 2), justification='center', font='Cardamon 15')],
        [sg.Text("Deg of pointer: ", justification='center', font='Cardamon 15'),
         sg.Text(" ", size=(35, 2), key="Degree", justification='center', font='Cardamon 15')],
        [sg.Button("Back", key='_BackScript_', size=(5, 1), font='Cardamon 15')],
        [sg.Button('Begin executing', key='_Run_', size=(60, 2), font='Cardamon 15')]
    ]


    layout_script = [[sg.Column(file_viewer),
                      sg.HSeparator(),
                      sg.Column(file_description)]]

    layout = [[sg.Column(layout_main, key='COL1', ),
               sg.Column(layout_manualstepper, key='COL2', visible=False),
               sg.Column(layout_painter, key='COL3', visible=False),
               sg.Column(layout_calib, key='COL4', visible=False),
               sg.Column(layout_script, key='COL5', visible=False),
               sg.Column(layout_calc, key='COL6', visible=False)]]

    global window
    # Create the Window
    window = sg.Window(title='Control System of motor-based machine', element_justification='c', layout=layout,
                       size=(1500, 700), location=(100, 100))
    canvas = window['canvas']
    execute_list = []
    empty_list = []

    file_size = ''
    calib_page = 0

    global PaintActive
    # Main loop
    while True:
        event, values = window.read()
        if event == sg.WIN_CLOSED:
            break
        event_handler(event, values, window, s, execute_list,  file_path, calib_page)

    window.close()


def event_handler(event, values, window, s, execute_list, file_path, calib_page):
    global file_name_to_execute
    global execute_index
    if event == "_ManualStepper_":
        send_state('m')  # manual stepper
        show_window(2, window)

    elif event == "_Rotation_":
        send_state('A')  # Auto Rotate

    elif event == "_Stop_":
        send_state('M')  # Stop

    elif event == "_JoyStickCrtl_":
        send_state('J')  # Joystick Control

    elif event == "_Painter_":
        handle_painter(window)

    elif event == "_Calib_":
        calib_page = 1
        handle_calibration(window, calib_page)

    elif event == "_Script_":
        upload_index = 0
        send_state('s')  # Script
        show_window(5, window)

    elif event == '_Folder_':
        handle_folder_selection(window, values)

    elif event == '_FileList_':
        handle_file_selection(window, values)

    elif event == '_upload_':
        handle_upload(window, file_path, execute_list)

    elif event == '_ExecutedList_':

        execute_index, file_name_to_execute = handle_exe_lst_sel(values, execute_list)
        if file_name_to_execute:
            window['_FileName_'].update(f'"{file_name_to_execute}" execute window')
        else:
            sg.popup_error("No file selected for execution.")

    elif event == '_Execute_':
        handle_execute(window, s)


    elif event == sg.WIN_CLOSED:
        window.close()

    elif "_BackMenu_" in event:
        send_state('B')
        show_window(1, window)

    elif "_BackScript_" in event:
        show_window(5, window)


def handle_manual_stepper(window):
    """
    Manages the manual control of a motor-based machine through a GUI, allowing the user to
    rotate the motor, stop it, or switch to joystick control. The function listens for
    specific button events and sends corresponding commands to the machine.
    """
    # Send the command to enter manual stepper mode
    send_state(STATES['manual'])

    # Show the window layout for manual stepper control
    show_window(2, window)

    while True:
        # Read the event and values from the GUI window
        event, values = window.read()

        # If the back button is pressed, break the loop and return to the main menu
        if event == "_BackMenu_":
            break
        # If the rotation button is pressed, send the auto-rotate command
        elif event == "_Rotation_":
            send_state(STATES['auto_rotate'])
        # If the stop button is pressed, send the stop command
        elif event == "_Stop_":
            send_state(STATES['stop'])
        # If the joystick control button is pressed, send the joystick control command
        elif event == "_JoyStickCrtl_":
            send_state(STATES['joystick'])

    # Once the loop ends, return to the main window layout
    show_window(1, window)


def handle_painter(window):
    """
    Manages the painter mode in a GUI application where a joystick controls a virtual paintbrush.
    The function sets up the paint environment, reads joystick data, and moves the cursor
    according to the joystick input. It runs in a loop until the painter mode is exited.
    """
    global PaintActive
    global painter_state

    # Indicate that the painter mode is active
    PaintActive = 1
    print('begin painter')
    # Send the state 'P' to indicate the start of painter mode
    send_state(STATES['painter'])

    # Start the painting application in a separate thread
    paint_thread = threading.Thread(target=Paint)
    paint_thread.start()
    last_data = '0'
    prev_Vx, prev_Vy = 481  , 474  # Initialize default joystick values
    # s.reset_input_buffer()
    while PaintActive:  # Continue while the painter mode is active
        if s.in_waiting > 0:
            # Fetch joystick data for the x and y coordinates
            Vx, Vy, painter_state ,last_data = fetch_joystick_data(painter_state, last_data)
            send_state('R')
            s.reset_input_buffer()
            # Calculate the change in position
            if Vx > 1100 or Vy > 1100:
                Vx , Vy = 481, 474
            dx, dy = Vx - 481, Vy - 474

            # Get the current mouse position
            curr_x, curr_y = mouse.get_position()

            # Move the cursor based on the joystick input
            mouse.move(curr_x - int(dy / 300), curr_y + int(dx / 300))
            # Update previous joystick values
            prev_Vx, prev_Vy = Vx, Vy


    # When painter mode is exited, return to the main window layout
    show_window(1, window)


def handle_calibration(window, calib_page):
    """
    Manages the calibration mode for a stepper motor in a GUI application.
    The function sets up the calibration environment, reads user input to start or stop
    the calibration process, and updates the GUI with the current calibration status.
    It runs in a loop until the calibration mode is exited.
    """
    # Send the state 'C' to indicate the start of calibration mode
    send_state(STATES['calibration'])

    # Show the calibration window layout
    show_window(4, window)
    s.reset_input_buffer()  # Reset RX buffer at the start

    while True:
        # Wait for an event and the corresponding values from the GUI
        event, values = window.read(timeout=1000)  # timeout to allow periodic checks for serial data
        print(event)

        # Handle the back button event to exit calibration mode
        if event == "_BackMenu_1":
            send_state('B')
            calib_page = 0
            break

        # Check if the calibration page is active
        if calib_page == 1:
            print('hello 1')

            if s.in_waiting > 0:
                    # Read the step count and phi from the serial buffer
                    data = s.read_until(b'\n').decode('utf-8').strip()
                    print(data)
                    if ',' in data:
                        step_count, phi = map(int, data.split(','))
                        # Update the GUI with the received step count
                        window["Counter"].update(value=f"{step_count}")
                        s.reset_input_buffer()  # Reset RX buffer at the start
                        phi = float(phi/ (1000))
                        phi = round(phi, 3)
                        # Update the GUI with the received phi value
                        window["Phi"].update(value=f"{phi}[deg]")
                        print(f"Received Counter: {step_count}, phi: {phi}")
                    else:
                        print(f"Unexpected data received: {data}")

    # When the calibration mode is exited, return to the main window layout
    show_window(1, window)


def handle_folder_selection(window, values):
    """
    Handles the selection of a folder in the GUI application.
    This function updates the file list in the GUI based on the selected folder
    and prepares the application for file operations within that folder.
    """
    global file_path

    # Get the path of the selected folder from the GUI values
    selected_folder = values['_Folder_']

    try:
        # Clear any previously displayed file content and header in the GUI
        window["_FileContent_"].update('')
        window["_FileHeader_"].update('')
        # List all files in the selected folder
        file_list = os.listdir(selected_folder)
    except Exception as e:
        # If an error occurs (e.g., the folder doesn't exist), set file_list to an empty list
        file_list = []

    # Filter the file list to only include .txt files
    fnames = [f for f in file_list if
              os.path.isfile(os.path.join(selected_folder, f)) and f.lower().endswith(".txt")]

    # Update the file list in the GUI with the filtered filenames
    window["_FileList_"].update(fnames)


def handle_file_selection(window, values):
    """
    Handles the selection of a file within the selected folder in the GUI application.
    This function reads the selected file's content and displays it in the GUI.
    It also updates the GUI with the file's name and size.
    """
    global file_name
    global file_path  # Ensure this is global if needed elsewhere

    try:
        # Construct the full file path using the selected folder and file name
        file_path = os.path.join(values["_Folder_"], values["_FileList_"][0])

        # Get the size of the selected file
        file_size = path.getsize(file_path)

        try:
            # Open the selected file in text mode with UTF-8 encoding and read its content
            with open(file_path, "rt", encoding='utf-8') as f:
                file_content = f.read()
        except Exception as e:
            # If there's an error reading the file, print the error message
            print("Error: ", e)

        # Get the selected file's name from the list of files
        file_name = values["_FileList_"][0]

        # Update the GUI with the file name and size
        window["_FileHeader_"].update(f"File name: {file_name} | Size: {file_size} Bytes")

        # Display the file content in the multiline text box in the GUI
        window["_FileContent_"].update(file_content)
    except Exception as e:
        # If there's an error in any of the above steps, print the error message
        print("Error: ", e)
        # Clear the file content display in the GUI
        window["_FileContent_"].update('')


def handle_upload(window, file_path, execute_list):
    """
    Manages the process of uploading a script file to the MSP430 microcontroller.
    It reads the file, translates its content, appends a file identifier, and then
    sends it through the serial interface. The function also updates the GUI to
    acknowledge the upload.
    """
    global upload_index
    s.reset_input_buffer()  # Clear the serial input buffer
    send_state(f"{file_name}\n")  # Send the file name to the MSP430
    print(file_name)  # Print the file name for debugging purposes
    execute_list.append(f"{file_name}")  # Add the file name to the execution list
    print(execute_list)  # Print the updated execution list for debugging purposes
    time.sleep(0.1)  # Short delay to ensure the file name is sent

    try:
        with open(file_path, "rb") as f:  # Open the file in binary read mode
            file_content_b = f.read()  # Read the file content as bytes
            print(file_content_b)  # Print the raw binary content for debugging
            file_content_str = translate_file(file_content_b)  # Translate the binary content to a string
            if upload_index == 0:
                ptr_state = 'f'  # Use 'f' as the identifier for the first file
            elif upload_index == 1:
                ptr_state = 'g'  # Use 'g' as the identifier for the second file
            elif upload_index == 2:
                ptr_state = 'h'  # Use 'h' as the identifier for the third file
            file_content_str += ptr_state  # Append the file identifier to the translated content
            print(file_content_str)  # Print the final string to be sent for debugging
            send_state(file_content_str, file_option=False)  # Send the string content to the MSP430
            print('sent file')  # Log that the file has been sent
    except Exception as e:
        print("Error: ", e)  # Print any error that occurs during file handling

    print(f"{file_size}")  # Print the size of the file for debugging

    time.sleep(0.5)  # Short delay to ensure the file content is sent

    upload_index += 1  # Increment the upload index for the next file

    while not s.in_waiting > 0:
        continue  # Wait until data is received from the MSP430

    window['_Acknowledge_'].update(f'"{file_name}" uploaded successfully!')  # Update the GUI to acknowledge the successful upload
    window.refresh()  # Refresh the GUI to reflect the changes

    print("upload file index: " + ptr_state)  # Print the current file index for debugging
    time.sleep(0.3)  # Short delay to ensure the acknowledgment is processed
    window['_ExecutedList_'].update(execute_list)  # Update the GUI with the list of uploaded files


def handle_exe_lst_sel(values, execute_list):
    # Retrieve the selected file name from the list
    if "_ExecutedList_" in values and values["_ExecutedList_"]:
        file_name_to_execute = values["_ExecutedList_"][0]
        print(f" file to execute {file_name_to_execute}")
    else:
        return None, None

    # Get the index of the selected file in the execution list
    if file_name_to_execute in execute_list:
        file_index = execute_list.index(file_name_to_execute)
        # Find the corresponding execution state
        execute_index = list(EXECUTE_INDICES.keys())[list(EXECUTE_INDICES.values()).index(file_index)]
        print(f"execute_index is {execute_index}")
        return execute_index, file_name_to_execute
    else:
        return None, None


def handle_execute(window, s):
    """
    Responsible for executing a selected script file by communicating through serial interface.
     It manages the execution process, handles user interactions, and updates the GUI accordingly.
    """
    if not file_name_to_execute:
        sg.popup_error("No file selected for execution.")  # Show an error if no file is selected for execution
        return
    show_window(6, window)  # Switch to the execution window layout
    window['_FileName_'].update(f'"{file_name_to_execute}" execute window')  # Update the window with the selected file name
    s.reset_input_buffer()  # Clear the serial input buffer
    s.reset_output_buffer()  # Clear the serial output buffer
    wait_data = 0  # Initialize a flag to wait for data from the MSP430
    executing = 1  # Set the executing flag to true

    while executing:
        s.reset_input_buffer()  # Clear the serial input buffer at the beginning of each loop iteration

        if not wait_data:
            event, values = window.read(timeout=1000)  # Wait for user interaction or a timeout
            if event == "_BackScript_":
                send_state('B')  # Send a 'B' state to the MSP430 to indicate a back request
                executing = 0  # Stop the execution loop
            elif event == '_Run_':
                send_state(execute_index)  # Send the selected script index to the MSP430
                print("Executing file index: " + execute_index)  # Log the execution of the script
            print(event)  # Print the event for debugging purposes

        if s.in_waiting > 0:
            wait_data = 1  # If data is waiting in the serial buffer, set the flag to wait for data

        if wait_data:
            data = s.read_until(b'\n').decode('utf-8').strip()  # Read the data from the serial port until a newline
            print(data)  # Print the received data for debugging purposes
            if data == "R":
                wait_data = 0  # If the data is "R", reset the wait_data flag
                time.sleep(1) # to show final degree before it changes
                window["Degree"].update(value="Script run continues")  # Update the degree value in the GUI
                window.refresh()  # Refresh the GUI to reflect the changes
            else:
                curr_phi = int(data) * 360 / 2036  # Calculate the degree based on the received data
                curr_phi = int(round(curr_phi, 0))  # Round the degree to the nearest integer
                window["Degree"].update(value=str(curr_phi) + "[deg]")  # Update the degree value in the GUI
                window.refresh()  # Refresh the GUI to reflect the changes
                s.reset_input_buffer()  # Clear the serial input buffer

        print('nothing to display')  # Log that there's nothing to display
    show_window(5, window)  # Return to the script window layout



'''
##########################################################################
                   Communication with MSP430
##########################################################################
'''


def show_window(layout_num, window):
    """
    Updates the visibility of window elements based on the specified layout number.
    """
    for i in range(1, 7):
        # Check if the current number matches the specified layout number
        if i == layout_num:
            # Make the layout corresponding to layout_num visible
            window[f'COL{layout_num}'].update(visible=True)
        else:
            # Hide the layout that does not match the layout_num
            window[f'COL{i}'].update(visible=False)


def send_state(message=None, file_option=False):
    """
    Sends a message to the serial port. The message can be either a byte sequence or a string.
    """
    # Reset the output buffer of the serial port to clear any previous data
    s.reset_output_buffer()
    # Check if file_option is True
    if file_option:
        # If file_option is True, use the message directly as bytes
        bytesMenu = message
    else:
        # If file_option is False, convert the message from ASCII string to bytes
        bytesMenu = bytes(message, 'ascii')

    # Write the bytesMenu to the serial port
    s.write(bytesMenu)


def fetch_joystick_data(change_painter_state, last_data):
    """
    Retrieves and processes data from the serial port for the joystick position.
    """
    # Read the incoming data until newline
    data = s.read_until(b'\n').decode('utf-8').strip()
    print(data)

    if 'P' in data:

        return 481, 474, 0,  last_data
    elif 'E' in data:
        return 481, 474, 1, last_data
    elif 'N' in data:
        return 481, 474, 2, last_data
    elif '\x00' in data:
        return 481, 474, change_painter_state, last_data
    try:
        # Split the line into x_value and y_value using the comma
        x_value, y_value = map(int, data.split(','))
        print(x_value, y_value)
    except ValueError:
        x_value, y_value = 481, 474

    return x_value, y_value, change_painter_state, last_data

def translate_file(input_data):
    global file_path  # Ensure this is global if needed elsewhere
    # Map of commands to their corresponding operation codes
    command_to_opcode = {
        'inc_lcd': '01',  # Increment LCD
        'dec_lcd': '02',  # Decrement LCD
        'rra_lcd': '03',  # Rotate right LCD
        'set_delay': '04',  # Set delay
        'clear_lcd': '05',  # Clear LCD
        'stepper_deg': '06',  # Stepper motor degrees
        'stepper_scan': '07',  # Stepper motor scan
        'sleep': '08'  # Sleep command
    }

    # List to store each translated command
    output_lines = []

    # Decode byte data to string and remove any extra spaces
    decoded_str = input_data.decode('utf-8').strip()
    # Split the decoded string into separate lines
    split_lines = decoded_str.split('\r\n')

    # Iterate through each line in the file
    for single_line in split_lines:
        # Split each line into the command and associated data
        elements = single_line.split()
        cmd = elements[0]  # First element is expected to be the command

        # Check if the command exists in the operation code map
        if cmd in command_to_opcode:
            opcode = command_to_opcode[cmd]  # Retrieve the operation code
            if len(elements) > 1:
                # If there's associated data with the command
                data_segment = elements[1]  # The data part
                if ',' in data_segment:
                    # For commands with multiple data points separated by commas
                    hex_data = ''.join([f'{int(value):02X}' if value.isdigit() else f'{ord(value):02X}' for value in data_segment.split(',')])
                else:
                    # For single data point commands, convert data to hexadecimal
                    if data_segment.isdigit():
                        hex_data = f'{int(data_segment):02X}'
                    else:
                        hex_data = f'{ord(data_segment):02X}'

                # Combine operation code and data
                translated_cmd = opcode + hex_data
            else:
                # Only the operation code if there's no data
                translated_cmd = opcode

            # Add the translated command to the output list
            output_lines.append(translated_cmd)

    # Join all translated lines into a single string with newline characters
    translated_content = '\n'.join(output_lines)

    # Return the translated content as a string (or you can convert to bytes if needed)
    return translated_content




'''
##########################################################################
                            main loop for FSM
##########################################################################
'''

if __name__ == '__main__':

    s = ser.Serial('COM7', baudrate=9600, bytesize=ser.EIGHTBITS,
                   parity=ser.PARITY_NONE, stopbits=ser.STOPBITS_ONE,
                   timeout=1)  # timeout of 1 sec so that the read and write operations are blocking,
    # after the timeout the program continues
    s.flush()  # clear the port
    enableTX = True
    s.set_buffer_size(1024, 1024)
    # clear buffers
    s.reset_input_buffer()
    s.reset_output_buffer()
    firstTime = 1
    PaintActive = 0

    GUI()