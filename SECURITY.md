# Security Policy

## Supported Versions

This project is currently in active development. Security updates are provided for the latest version on the main branch.

| Version | Supported          |
| ------- | ------------------ |
| main    | :white_check_mark: |
| older   | :x:                |

## Reporting a Vulnerability

If you discover a security vulnerability in this project, please report it by:

1. **Do NOT** create a public GitHub issue
2. Send an email to the repository maintainer through GitHub
3. Include the following information:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if available)

### What to Expect

- **Initial Response**: You will receive an acknowledgment within 48 hours
- **Updates**: You will be kept informed of the progress toward a fix
- **Resolution**: Once a fix is available, we will notify you before making it public
- **Credit**: If desired, you will be credited for the discovery in the fix announcement

## Security Best Practices

When deploying this backend server:

- Always use the latest version from the main branch
- Keep all dependencies (oatpp, system libraries) up to date
- Review and customize the CORS settings in `CorsInterceptor.hpp`
- Use secure WebSocket connections (wss://) in production
- Implement proper authentication and authorization for your use case
- Run the server behind a reverse proxy (e.g., nginx) in production
- Follow the principle of least privilege for system permissions

## Known Security Considerations

- JWT token validation is implemented - ensure proper secret key management
- BLE service requires appropriate system permissions
- MQTT connections should use TLS in production environments
