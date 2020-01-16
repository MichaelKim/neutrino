const path = require('path');
const webpack = require('webpack');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const TerserPlugin = require('terser-webpack-plugin');

const rendererConfig = {
  entry: './src/renderer/index.jsx',
  output: {
    filename: 'renderer.js'
  },
  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'babel-loader',
            options: {
              presets: [
                [
                  '@babel/preset-env',
                  {
                    targets: '>1%, not ie 11, not op_mini all'
                  }
                ]
              ]
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
    },
    extensions: ['.js', '.jsx']
  },
  node: {
    fs: 'empty',
    __dirname: false
  },
  stats: {
    colors: true
  },
  devServer: {
    contentBase: [path.join(__dirname, 'dist/renderer')],
    host: 'localhost',
    port: '8080',
    hot: true,
    overlay: true
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
    rendererConfig.plugins.push(
      new webpack.DefinePlugin({
        'process.env.PRODUCTION': JSON.stringify(true)
      })
    );
  } else {
    rendererConfig.mode = 'development';
    rendererConfig.devtool = '#cheap-module-source-map';
    rendererConfig.plugins.push(
      new webpack.DefinePlugin({
        'process.env.PRODUCTION': JSON.stringify(false)
      })
    );
  }

  return rendererConfig;
};
