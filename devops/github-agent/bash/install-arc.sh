helm upgrade --install arc-runner-set \
  --namespace arc-runners \
  -f values.yaml \
  oci://ghcr.io/actions/actions-runner-controller-charts/gha-runner-scale-set
