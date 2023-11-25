#!/bin/bash
# ELEC377 - Operating System
# Lab 4 - Shell Scripting, ps.sh
# Program Description: Script to list running processes using /proc interface.

# Variables to hold command line options
showRSS="no"
showComm="no"
showCommand="no"
showGroup="no"

# Parse command line arguments
#+
# Function: Argument Parsing
#
# Purpose: This section parses the command line arguments passed to the script. It sets flags that determine
# what additional process information should be displayed in the output.
#
# Parameters: Command line arguments
#
# Returns: None. Sets global variables to control script behavior.
#-

while [[ $# -gt 0 ]]; do
    case "$1" in
        -rss)
            showRSS="yes"
            ;;
        -comm)
            if [[ $showCommand == "yes" ]]; then
                echo "Error: -comm and -command cannot both be specified."
                exit 1
            fi
            showComm="yes"
            ;;
        -command)
            if [[ $showComm == "yes" ]]; then
                echo "Error: -comm and -command cannot both be specified."
                exit 1
            fi
            showCommand="yes"
            ;;
        -group)
            showGroup="yes"
            ;;
        *)
            echo "Error: Unknown flag $1"
            exit 1
            ;;
    esac
    shift
done

tmpfile="/tmp/tmpPs$$.txt"
# Ensure the temporary file does not exist
rm -f "$tmpfile"

# Dynamic header creation
header="PID"
[[ $showGroup == "yes" ]] && header="$header\tGROUP"
[[ $showRSS == "yes" ]] && header="$header\tRSS"
[[ $showComm == "yes" || $showCommand == "yes" ]] && header="$header\tCOMMAND"
echo -e "$header" > "$tmpfile"

#+
# Function: Process Information Collection
#
# Purpose: Loops over each process directory in /proc, collects requested data, 
# and writes formatted output to a temporary file. Handles missing information gracefully.
#
# Parameters: None. Utilizes global flags for conditional data collection.
#
# Returns: None. Outputs to a temporary file which is sorted before presentation.
#-

for p in /proc/[0-9]*; do
    if [[ -d "$p" ]]; then  # Check if the directory exists
        pid=$(basename "$p")

        # Extract the user and group ID
        if [[ -f "$p/status" ]]; then
            user_id=$(grep 'Uid:' "$p/status" | awk '{print $2}')
            group_id=$(grep 'Gid:' "$p/status" | awk '{print $2}')
        else
            echo "Warning: /proc/$pid/status does not exist, skipping..." >&2
            continue
        fi

        # Convert user ID to name
        user_name=$(grep ":x:$user_id:" /etc/passwd | cut -d: -f1)
        if [[ -z "$user_name" ]]; then
            user_name="UNKNOWN"
        fi

        # Convert group ID to name
        group_name=$(grep ":x:$group_id:" /etc/group | cut -d: -f1)
        if [[ -z "$group_name" ]]; then
            group_name="UNKNOWN"
        fi

        # Extract RSS if required
        if [[ $showRSS == "yes" ]]; then
            rss=$(grep 'VmRSS:' "$p/status" | awk '{print $2}')
            rss=${rss:-0}  # Default to 0 if RSS is not found
        else
            rss=""
        fi

# Extract the process name if required
#+
# Function: Process Name Extraction
#
# Purpose: To retrieve the command name of the process from the /proc/[PID]/comm file.
# If the comm file does not exist, the name is used as a fallback.
#
# Parameters: None. Operates on current process directory within the loop.
#
# Returns: Sets the 'comm' variable with the process name.
#-
        
        if [[ $showComm == "yes" ]]; then
            if [[ -f "$p/comm" ]]; then
                comm=$(cat "$p/comm")
            else
                echo "Warning: /proc/$pid/comm does not exist, using name..." >&2
                comm=$name
             fi
        fi
        
# Extract the command line if required
#+
# Function: Command Line Extraction
#
# Purpose: To retrieve the full command line for the process from the /proc/[PID]/cmdline file.
# If the cmdline file does not exist, the name is used as a fallback.
#
# Parameters: None. Operates on current process directory within the loop.
#
# Returns: Sets the 'cmdline' variable with the full command line or name as fallback.
#-

        if [[ $showCommand == "yes" ]]; then
            if [[ -f "$p/cmdline" ]]; then
                cmdline=$(tr '\0' ' ' < "$p/cmdline" | sed 's/ \{2,\}/ /g')
                # If cmdline is empty, use Name for the command
                if [ -z "$cmdline" ]; then
                    cmdline=$name
                fi
            else
                echo "Warning: /proc/$pid/cmdline does not exist, using name..." >&2
                 cmdline=$name
            fi
        fi

# Truncate command line if necessary
#+
# Function: Command Line Truncation
#
# Purpose: Truncates the command line to a maximum length to ensure the output is readable. 
# If the 'comm' or 'command' flags are set, it will select the appropriate field for display.
#
# Parameters: None. Utilizes global flags and variables.
#
# Returns: Sets the 'display_field' variable with the truncated or full command as appropriate.
#-

        display_field=$name  # Default display field is the process name
        if [ $showComm == "yes" ]; then
            display_field=$comm
        elif [ $showCommand == "yes" ]; then
            display_field=$cmdline
            max_cmd_length=50
            if [ ${#display_field} -gt $max_cmd_length ]; then
                display_field="${display_field:0:$max_cmd_length}..."
            fi
        fi

# Conditional column printing
#+
# Function: Output Formatting
#
# Purpose: Formats the output by printing the collected data into the temporary file. 
# This includes the process ID, group, RSS, and command as determined by the user flags.
#
# Parameters: None. Utilizes global flags and variables set previously.
#
# Returns: None. Writes to the temporary file for later sorting and display.
#-

printf "%d" "$pid" >> "$tmpfile"
[[ $showGroup == "yes" ]] && printf "\t%s" "$group_name" >> "$tmpfile"
[[ $showRSS == "yes" ]] && printf "\t%s" "$rss" >> "$tmpfile"
[[ $showComm == "yes" || $showCommand == "yes" ]] && printf "\t%s" "$display_field" >> "$tmpfile"
printf "\n" >> "$tmpfile"
    else
        echo "Warning: Directory $p no longer exists, skipping..." >&2
    fi
done

# Conditional column printing
#+
# Function: Output Formatting
#
# Purpose: Formats the output by printing the collected data into the temporary file. 
# This includes the process ID, group, RSS, and command as determined by the user flags.
#
# Parameters: None. Utilizes global flags and variables set previously.
#
# Returns: None. Writes to the temporary file for later sorting and display.
#-
sort -n "$tmpfile"
