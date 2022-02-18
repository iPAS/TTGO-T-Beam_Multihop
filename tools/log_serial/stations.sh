export stations='
5196 ปภ
4968 ป่าเมี่ยง
4964 กปส
4948 เจดีย์
5340 rogue'

SAVED_IFS=$IFS   # Save current IFS (Internal Field Separator)
IFS=$'\n'      # Change IFS to newline char
stations=($stations) # split the `names` string into an array by the same name
IFS=$SAVED_IFS   # Restore original IFS

