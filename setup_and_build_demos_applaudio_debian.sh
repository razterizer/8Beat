#!/bin/bash

REPO_DIR="8Beat"
BUILD_DIR="demos"

sudo apt install pkg-config

# sudo apt install libopenal-dev

sudo apt install libasound2-dev

sudo apt install libsndfile-dev

cd ..

# Define the required folder name
REQUIRED_NAME="lib"
# Get the current folder name
CURRENT_NAME=$(basename "$PWD")
# Check if the current folder name matches the required name
if [[ "$CURRENT_NAME" != "$REQUIRED_NAME" ]]; then
  echo -e "${YELLOW}Warning:${RESET} You are not in the correct folder. It is highly recommended that you check out the 8Beat repo in a folder named '$REQUIRED_NAME'. The demos will run fine but other repos might expect to find it there."
  # exit 1
fi

./"${REPO_DIR}"/fetch-dependencies.py "${REPO_DIR}"/dependencies

cd "${REPO_DIR}/${BUILD_DIR}"

./build_all_demos_applaudio.sh

while true; do
    # Ask the user
    read -p "Do you want to run the program? (yes/no): " response

    # Process the response
    case "$response" in
        yes|y|Y|YES|Yes)
            echo "Running the program..."
            ./run_all_demos.sh
            break
            ;;
        no|n|N|NO|No)
            echo "Alright. Have a nice day!"
            break
            ;;
        *)
            echo "Invalid response. Please answer yes or no."
            ;;
    esac
done

