const path = require('path');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const TerserPlugin = require('terser-webpack-plugin');

const rendererConfig = {
  entry: './src/renderer/index.jsx',
  output: {
    path: path.resolve('./dist/neutrino-renderer'),
    filename: 'renderer.js',
    clean: true
  },
  target: 'web',
  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'babel-loader',
            options: {
              targets: '> 1%, not ie 11',
              presets: ['@babel/preset-env']
            }
          }
        ]
      }
    ]
  },
  plugins: [
    new MiniCssExtractPlugin({
      filename: '[name].css',
      chunkFilename: '[id].css'
    })
  ],
  resolve: {
    alias: {
      fs: 'neutrinojs/lib/fs'
    }
  },
  node: {
    __dirname: false
  },
  stats: {
    colors: true
  }
};

module.exports = (env, argv) => {
  rendererConfig.output.path = env.path;

  if (argv.mode === 'production') {
    rendererConfig.mode = 'production';
    rendererConfig.optimization = {
      minimizer: [
        new TerserPlugin({
          extractComments: false,
          terserOptions: {
            compress: {
              drop_console: true
            }
          }
        })
      ]
    };
  } else {
    rendererConfig.mode = 'development';
    rendererConfig.devtool = 'cheap-module-source-map';
    rendererConfig.devServer = {
      static: path.resolve('./build'),
      host: 'localhost',
      port: 8080,
      hot: true
    };
  }

  return rendererConfig;
};
