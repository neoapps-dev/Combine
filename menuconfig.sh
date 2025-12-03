#!/bin/bash
: '
   Copyright 2025 NEOAPPS

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
'
CONFIG_FILE="build.conf"
RENDERERS_DIR="include/renderers"
SCRIPTS_DIR="include/scripting"
get_available_renderers() {
  local renderers=()
  for file in "$RENDERERS_DIR"/*Renderer.h; do
    if [ -f "$file" ]; then
      local name=$(basename "$file" Renderer.h)
      renderers+=("$name")
    fi
  done
  echo "${renderers[@]}"
}

get_available_scripts() {
  local scripts=()
  for file in "$SCRIPTS_DIR"/*Engine.h; do
    if [ -f "$file" ]; then
      local name=$(basename "$file" Engine.h)
      scripts+=("$name")
    fi
  done
  echo "${scripts[@]}"
}

load_config() {
  if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
  else
    local available_renderers=($(get_available_renderers))
    local available_scripts=($(get_available_scripts))
    RENDERER="${available_renderers[0]:-OpenGL}"
    SUPPORTED_SCRIPTS="${available_scripts[0]:-ChaiScript}"
    INIT_SCRIPT="scripts/init.chai"
    DEBUG="yes"
    OPTIMIZATIONS="yes"
    STATIC="no"
  fi
}

save_config() {
  cat >"$CONFIG_FILE" <<EOF
RENDERER=$RENDERER
SUPPORTED_SCRIPTS=$SUPPORTED_SCRIPTS
INIT_SCRIPT=$INIT_SCRIPT
DEBUG=$DEBUG
OPTIMIZATIONS=$OPTIMIZATIONS
STATIC=$STATIC
EOF
  echo "-> $CONFIG_FILE"
}

select_renderer() {
  local renderers=($(get_available_renderers))
  local menu_items=()
  for r in "${renderers[@]}"; do
    local status="off"
    [ "$r" = "$RENDERER" ] && status="on"
    menu_items+=("$r" "${r} renderer" "$status")
  done

  if [ ${#menu_items[@]} -eq 0 ]; then
    dialog --title "Error" --msgbox "No renderers found in $RENDERERS_DIR" 8 50
    return
  fi

  result=$(dialog --title "Select Renderer" \
    --radiolist "Choose rendering backend:" 15 50 ${#renderers[@]} \
    "${menu_items[@]}" \
    2>&1 >/dev/tty)

  if [ $? -eq 0 ] && [ -n "$result" ]; then
    RENDERER="$result"
  fi
}

select_scripts() {
  local scripts=($(get_available_scripts))
  local menu_items=()
  for s in "${scripts[@]}"; do
    local status="off"
    [[ "$SUPPORTED_SCRIPTS" == *"$s"* ]] && status="on"
    menu_items+=("$s" "${s} language" "$status")
  done

  if [ ${#menu_items[@]} -eq 0 ]; then
    dialog --title "Error" --msgbox "No script engines found in $SCRIPTS_DIR" 8 50
    return
  fi

  result=$(dialog --title "Select Script Engines" \
    --checklist "Choose scripting backends:" 15 50 ${#scripts[@]} \
    "${menu_items[@]}" \
    2>&1 >/dev/tty)

  if [ $? -eq 0 ]; then
    SUPPORTED_SCRIPTS=$(echo "$result" | tr ' ' ',')
  fi
}

set_init_script() {
  result=$(dialog --title "Init Script" \
    --inputbox "Enter path to init script:" 8 60 "$INIT_SCRIPT" \
    2>&1 >/dev/tty)

  if [ $? -eq 0 ] && [ -n "$result" ]; then
    INIT_SCRIPT="$result"
  fi
}

toggle_debug() {
  if [ "$DEBUG" = "yes" ]; then
    DEBUG="no"
  else
    DEBUG="yes"
  fi
}

toggle_optimizations() {
  if [ "$OPTIMIZATIONS" = "yes" ]; then
    OPTIMIZATIONS="no"
  else
    OPTIMIZATIONS="yes"
  fi
}

toggle_static() {
  if [ "$STATIC" = "yes" ]; then
    STATIC="no"
  else
    STATIC="yes"
  fi
}

main_menu() {
  while true; do
    local debug_label="[ ]"
    local opt_label="[ ]"
    local static_label="[ ]"
    [ "$DEBUG" = "yes" ] && debug_label="[*]"
    [ "$OPTIMIZATIONS" = "yes" ] && opt_label="[*]"
    [ "$STATIC" = "yes" ] && static_label="[*]"
    choice=$(dialog --title "Combine Engine Configuration" \
      --menu "Use arrow keys to navigate, Enter to select:" 20 60 9 \
      "1" "Renderer:          $RENDERER" \
      "2" "Script Engines:    $SUPPORTED_SCRIPTS" \
      "3" "Init Script:       $INIT_SCRIPT" \
      "4" "$debug_label Debug Mode" \
      "5" "$opt_label Optimizations" \
      "6" "$static_label Static Linking" \
      "" "" \
      "S" "Save and Exit" \
      "Q" "Quit without Saving" \
      2>&1 >/dev/tty)
    case $? in
    1 | 255) break ;;
    esac
    case $choice in
    1) select_renderer ;;
    2) select_scripts ;;
    3) set_init_script ;;
    4) toggle_debug ;;
    5) toggle_optimizations ;;
    6) toggle_static ;;
    S)
      save_config
      break
      ;;
    Q) break ;;
    esac
  done
}

if ! command -v dialog &>/dev/null; then
  echo "Error: 'dialog' is not installed."
  echo "Install it with: sudo pacman -S dialog (Arch) or sudo apt install dialog (Debian/Ubuntu)"
  exit 1
fi

load_config
main_menu
clear
