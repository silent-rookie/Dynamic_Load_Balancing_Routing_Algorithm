NS3_VERSION="ns-3.31"

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip -n ${NS3_VERSION}.zip

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Rebuild whichever build is configured right now
./waf -j4 || exit 1
