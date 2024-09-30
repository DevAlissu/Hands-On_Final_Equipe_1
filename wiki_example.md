# Relatório de Desenvolvimento - Tarefa 1.1

### Especificação
- Buscar o código responsável pela tela de desbloqueio e encontrar a string "Enter your pattern".
- Encontrar o arquivo `strings.xml` que define esta string.
- Determinar o nome da string (campo `name` da tag `string`).
- Encontrar a tradução correspondente em português desta string.

### Procedimentos

#### 1. Encontrar o arquivo `strings.xml` que define a string "Enter your pattern"
- **Metodologia:** A dica do monitor Mateus foi crucial para essa etapa. Ele sugeriu que utilizássemos o Code Search para procurar a string "Enter your pattern", uma vez que essa string é visível na tela e possivelmente nos levaria ao arquivo `strings.xml` onde ela está definida.
- **Resultado:** Após a busca, encontramos o arquivo no seguinte caminho: `frameworks/base/packages/SystemUI/res-keyguard/values/strings.xml`.

![image](https://github.com/hendriomm/Hands-On-Android/assets/91330677/32c60e9f-8132-42b6-9145-454fe51e3ad3)

#### 2. Identificar o nome da string
- **Metodologia:** Com o arquivo `strings.xml` localizado, buscamos pela linha que contém a string "Enter your pattern" para identificar o valor do campo `name`.
- **Resultado:** O nome da string é: `"keyguard_enter_your_pattern"`.

![Captura de tela de 2024-07-09 20-53-09](https://github.com/hendriomm/Hands-On-Android/assets/91330677/d20a021d-cdce-4105-8f36-80ebbe45e677)

#### 3. Encontrar a tradução em português
- **Metodologia:** Utilizando o Code Search, inserimos o nome da string `"keyguard_enter_your_pattern"` para encontrar sua tradução. Este processo envolveu a análise dos resultados até localizar o arquivo correspondente em português.
- **Resultado:** A tradução foi encontrada no caminho: `frameworks/base/packages/SystemUI/res-keyguard/values-pt-rBR/strings.xml`. A string em português é: `<string name="keyguard_enter_your_pattern" msgid="351503370332324745">"Digite seu padrão"</string>`.

![Captura de tela de 2024-07-09 20-56-48](https://github.com/hendriomm/Hands-On-Android/assets/91330677/109dc573-6e14-431b-b57e-337d4ca74c94)

### Aluno Responsável

**Matheus Silva dos Santos**