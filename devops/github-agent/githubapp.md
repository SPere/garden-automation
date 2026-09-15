# Guide: Setting Up GitHub Actions Runner Controller (ARC) with a GitHub App in K3s

GitHub Apps are vastly superior for automated K3s infrastructure compared to standard Personal Access Tokens (PATs) for several key reasons:
* **No Expiration:** Unlike PATs, which expire and break pipelines, a GitHub App uses cryptographic keys.
* **Hyper-granular Permissions:** You only give it access to manage self-hosted runners on specific repositories.
* **Higher API Limits:** It is decoupled from your personal user account's API limits.

---

## Step 1: Create the GitHub App

1. Go to your GitHub profile/organization **Settings** → **Developer Settings** → **GitHub Apps** → **New GitHub App**.
2. Set the **App Name** to `k3s-arc-runner` (or anything unique).
3. Set the **Homepage URL** to any placeholder URL (e.g., `https://localhost`).
4. Under **Webhook**, uncheck **Active** (unless you plan to configure webhook-driven scaling later).
5. Configure the following **Permissions**:
   * **Repository Permissions** → **Actions:** `Read-only`
   * **Repository Permissions** → **Administration:** `Read & Write` (required to register self-hosted runners)
   * **Repository Permissions** → **Metadata:** `Read-only` (automatically added)
6. Click **Create GitHub App**.

---

## Step 2: Save the IDs and Private Key

Once the app is created, retrieve three critical pieces of information from its settings page:

1. **App ID:** Copy the App ID found near the top of the General tab.
2. **Private Key:** Scroll down to the *Private keys* section and click **Generate a private key**. A `.pem` file will download to your computer.
3. **Installation ID:** 
   * In the left sidebar, click **Install App**, select your account/organization, and click **Install**.
   * Once installed, look at your browser's URL bar. The last number in the URL is your Installation ID (e.g., `https://github.com`).

---

## Step 3: Map the App Credentials into K3s

On your Ubuntu server, run the following command to create a Kubernetes Secret holding your App ID, Installation ID, and the raw text content of your downloaded `.pem` key:

```bash
kubectl create secret generic arc-github-app-secret \
  --namespace=arc-runners \
  --create-namespace \
  --from-literal=github_app_id="YOUR_APP_ID" \
  --from-literal=github_app_installation_id="YOUR_INSTALLATION_ID" \
  --from-literal=github_app_private_key='-----BEGIN RSA PRIVATE KEY-----
  ... [Paste entire content of your downloaded .pem file here] ...
  -----END RSA PRIVATE KEY-----'
```

---

## Step 4: Deploy the Runner Scale Set Using the App

Create or update your Helm `values.yaml` file to tell ARC to use the newly created Kubernetes Secret for authentication.

```yaml
## values.yaml
githubConfigUrl: "https://github.com"

# Point to the secret created in Step 3 instead of using a token string
githubConfigSecret:
  name: arc-github-app-secret

## Enable Docker-in-Docker (dind)
containerMode:
  type: "dind"

maxRunners: 3
```

Apply this updated setup to your cluster with Helm:

```bash
helm upgrade --install arc-runner-set \
  --namespace arc-runners \
  -f values.yaml \
  oci://ghcr.io/actions/actions-runner-controller-charts/gha-runner-scale-set
```

The K3s agent controller will now transparently trade your private key for short-lived installation access tokens automatically, ensuring zero pipeline downtime from credential expiration.
