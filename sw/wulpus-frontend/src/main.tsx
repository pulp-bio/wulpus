import {
  QueryClient,
  QueryClientProvider
} from '@tanstack/react-query';
import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { Toaster } from 'react-hot-toast';
import { createBrowserRouter } from "react-router";
import { RouterProvider } from "react-router/dom";
import App from './App.tsx';
import './index.css';
import { LogsPage } from './LogsPage';

const router = createBrowserRouter([
  { path: '/', element: <App /> },
  { path: '/data', element: <LogsPage /> },
])

const queryClient = new QueryClient()

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <QueryClientProvider client={queryClient}>
      <RouterProvider router={router} />
      <Toaster position="top-center" />
    </QueryClientProvider>
  </StrictMode>,
)
