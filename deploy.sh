#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# virt-off-server deploy script
#
# Prerequisites:
#   - Yandex Cloud CLI (yc) installed and configured
#   - Docker installed
#   - You have created a Container Registry:
#       yc container registry create --name virt-off-server
#
# Usage:
#   ./deploy.sh
#
# The script will prompt for the registry ID on first run
# and save it to .deploy-config for reuse.
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/.deploy-config"

REGISTRY_ID=""

if [[ -f "$CONFIG_FILE" ]]; then
    source "$CONFIG_FILE"
fi

if [[ -z "$REGISTRY_ID" ]]; then
    echo "Enter your Yandex Container Registry ID:"
    echo "  (find it via: yc container registry list)"
    read -r -p "> " REGISTRY_ID

    if [[ -z "$REGISTRY_ID" ]]; then
        echo "ERROR: Registry ID is required."
        exit 1
    fi

    echo "REGISTRY_ID=$REGISTRY_ID" > "$CONFIG_FILE"
    echo "Saved to $CONFIG_FILE"
fi

REGISTRY="cr.yandex/$REGISTRY_ID"
IMAGE="$REGISTRY/virt-off-server:latest"

echo "=== Building image ==="
docker build -t "$IMAGE" "$SCRIPT_DIR"

echo "=== Pushing to registry ==="
docker push "$IMAGE"

echo "=== Creating / updating VM ==="
INSTANCE_NAME="virt-off-server"

if yc compute instance list --format json | grep -q "$INSTANCE_NAME"; then
    echo "Updating existing VM..."
    yc compute instance update-container "$INSTANCE_NAME" \
        --container-image="$IMAGE" \
        --container-restart-policy=always
else
    echo "Creating new VM..."
    yc compute instance create-with-container "$INSTANCE_NAME" \
        --container-image="$IMAGE" \
        --container-restart-policy=always \
        --zone=ru-central1-a \
        --platform=standard-v3 \
        --cores=2 \
        --memory=2GB \
        --network-interface=nat-ip-version=ipv4,subnet-name=default-ru-central1-a \
        --preemptible
fi

echo "=== Done ==="
echo "VM IP: $(yc compute instance get "$INSTANCE_NAME" --format json | grep -o '"one_to_one_nat":[^}]*' | grep -o '"ipv4_address":"[^"]*' | cut -d'"' -f4)"
