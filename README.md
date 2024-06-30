# i18next Scanner Configuration

This configuration is designed to be used with i18next-scanner for extracting translation keys from JavaScript and JSX files in a React project. The configuration will scan specified files, extract keys, and save them into JSON files in the appropriate locale directories.

## Table of Contents

- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Options](#options)
- [Custom Transform](#custom-transform)
- [Contribution](#contribution)

## Installation

First, install the required npm packages:

```sh
npm install i18next-scanner fs path
```

## Configuration

Create a configuration file (e.g., `i18next-scanner.config.js`) and copy the following content into it:

```javascript
const fs = require('fs');
const path = require('path');
const { Parser } = require('i18next-scanner');

module.exports = {
  input: [
    'app/javascript/**/*.{js,jsx}', // Include all JS and JSX files in app/javascript and subdirectories
  ],
  options: {
    debug: true,
    removeUnusedKeys: true,
    sort: true,
    lngs: ['en', 'de'], // Add other languages as needed
    resource: {
      loadPath: 'public/locales/{{lng}}/{{ns}}.json',
      savePath: 'public/locales/{{lng}}/{{ns}}.json',
      jsonIndent: 2,
    },
    ns: [
      'translation',
    ],
    defaultLng: 'en',
    defaultNs: 'translation',
    interpolation: {
      prefix: '{{',
      suffix: '}}',
    },
    trans: {
      component: 'Trans',
      i18nKey: 'i18nKey',
      extensions: ['.js', '.jsx'],
      fallbackKey: (ns, value) => value, // Use value as key
    },
    func: {
      list: ['i18next.t', 'i18n.t'],
      extensions: ['.js', '.jsx'],
    },
    keySeparator: false, // Use content as keys
    namespaceSeparator: false, // Namespace separator
    defaultValue: (lng, ns, key) => (lng === 'en' ? key : ''),
    skipDefaultValues: (lng) => lng !== 'en',
  },
  transform: function customTransform(file, enc, done) {
    const parser = this.parser;
    const content = file.contents.toString(enc);
    const lines = content.split('\n');

    // Open the output file for appending
    const outputFilePath = path.resolve(__dirname, 'output.txt');
    const outputStream = fs.createWriteStream(outputFilePath, { flags: 'a' });

    // Log the file path and number of lines
    outputStream.write(`${file.path}\n`);
    //outputStream.write(`Number of lines: ${lines.length}\n`);

    // Use the built-in parser for functions and Trans components
    parser.parseFuncFromString(content, { list: ['i18next.t', 'i18n.t'] });
    parser.parseTransFromString(content);

    // Use custom regex to extract string literals in JS/JSX files
    const regex = /(['"])((?:\\.|[^\\])*?)\1/g;
    const importRegex = /^\s*import\s+.*\s+from\s+['"].*['"];?$/;
    const classRegex = /\.([a-zA-Z0-9_-]+)\s*{/;
    const jsxContentRegex = /<[^>]+>([^<]+)<\/[^>]+>/g;

    let match;
    while ((match = regex.exec(content)) !== null) {
      const key = match[2]; // Extract the content inside quotes
      const precedingContent = content.substring(0, match.index).trim();

      // Skip unwanted content
      if (
        key.includes('@mui') || 
        key.includes('/') || 
        importRegex.test(precedingContent) || 
        classRegex.test(precedingContent) || 
        precedingContent.match(/className\s*=/) || 
        precedingContent.match(/class\s*=/) || 
        precedingContent.match(/styles\s*=\s*\{[^}]*\}/)
      ) {
        continue;
      }

      parser.set(key, key);

      // Find the line number
      const lineNumber = content.substring(0, match.index).split('\n').length;

      outputStream.write(`${lineNumber}\n`);
      //outputStream.write(`Content: ${key}\n`);
    }

    // Use custom regex to capture inner text content within JSX tags
    while ((match = jsxContentRegex.exec(content)) !== null) {
      const key = match[1].trim();
      if (key && !key.includes('@mui') && !key.includes('/') && key.length > 1) {
        const lineNumber = content.substring(0, match.index).split('\n').length;
        parser.set(key, key);

        outputStream.write(`${lineNumber}\n`);
        //outputStream.write(`Content: ${key}\n`);
      }
    }

    outputStream.end(done);
  },
};
```

## Usage

To run i18next-scanner with the provided configuration, use the following command:

```sh
npx i18next-scanner --config i18next-scanner.config.js
```

This command will scan the specified files and extract translation keys into the JSON files located in the `public/locales` directory.

## Options

The `options` object in the configuration contains various settings to customize the scanning and extraction process:

- **debug**: Enable debug mode.
- **removeUnusedKeys**: Remove keys that are not found in the source files.
- **sort**: Sort keys alphabetically.
- **lngs**: List of languages to support (e.g., `['en', 'de']`).
- **resource**: Configuration for loading and saving translation files.
  - **loadPath**: Path to load translation files.
  - **savePath**: Path to save translation files.
  - **jsonIndent**: Indentation level for JSON files.
- **ns**: List of namespaces to use.
- **defaultLng**: Default language.
- **defaultNs**: Default namespace.
- **interpolation**: Configuration for interpolation in translation strings.
- **trans**:
