# Anime CLI

Anime CLI is a command-line interface application that allows users to search for anime, select an anime from the search results, and choose episodes using the Consumet API. This project is designed to provide a simple and efficient way to explore anime content directly from the terminal.

## Features

- Search for anime titles using keywords.
- Select an anime from the search results.
- View and select episodes from the chosen anime.
- Utilizes the Consumet API for fetching anime data.

## Requirements

- GCC or any compatible C compiler
- Make
- cURL library (for API requests)
- JSON-C library (for parsing JSON responses)

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/anime-cli.git
   cd anime-cli
   ```

2. Build the project:
   ```
   make
   ```

3. Run the application:
   ```
   ./anime-cli
   ```

## Usage

After running the application, you will be prompted to enter a search query for anime. The application will display a list of matching anime titles. You can then select an anime to view its episodes.

## Running Tests

To run the unit tests for the API and UI components, use the following command:
```
make test
```

## Contributing

Contributions are welcome! If you have suggestions for improvements or new features, please open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.