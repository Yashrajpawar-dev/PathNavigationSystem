// ============================================================
// frontend/src/services/api.js
// Client API layer communicating with the Node.js Express bridge
// ============================================================

const API_BASE = '/api';

export async function fetchHealth() {
  const res = await fetch(`${API_BASE}/health`);
  if (!res.ok) throw new Error('Failed to fetch health');
  return res.json();
}

export async function fetchDashboard() {
  const res = await fetch(`${API_BASE}/dashboard`);
  if (!res.ok) throw new Error('Failed to fetch dashboard metrics');
  return res.json();
}
export const fetchDashboardStats = fetchDashboard;

export async function fetchLocations() {
  const res = await fetch(`${API_BASE}/locations`);
  if (!res.ok) throw new Error('Failed to fetch locations');
  return res.json();
}

export async function fetchConnections() {
  const res = await fetch(`${API_BASE}/connections`);
  if (!res.ok) throw new Error('Failed to fetch connections');
  return res.json();
}

export async function fetchRoute(fromId, toId, mode = 'distance') {
  const res = await fetch(`${API_BASE}/route?from=${fromId}&to=${toId}&mode=${mode}`);
  if (!res.ok) throw new Error('Failed to calculate route');
  return res.json();
}

export async function fetchAlternatives(fromId, toId, mode = 'distance') {
  const res = await fetch(`${API_BASE}/alternatives?from=${fromId}&to=${toId}&mode=${mode}`);
  if (!res.ok) throw new Error('Failed to calculate alternative routes');
  return res.json();
}

export async function fetchNearestFacility(fromId, category, mode = 'distance') {
  const res = await fetch(`${API_BASE}/nearest?from=${fromId}&category=${encodeURIComponent(category)}&mode=${mode}`);
  if (!res.ok) throw new Error('Failed to find nearest facility');
  return res.json();
}

export async function blockWalkway(fromId, toId) {
  const res = await fetch(`${API_BASE}/walkway/block`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ from: fromId, to: toId })
  });
  if (!res.ok) throw new Error('Failed to block walkway');
  return res.json();
}

export async function restoreWalkway(fromId, toId) {
  const res = await fetch(`${API_BASE}/walkway/restore`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ from: fromId, to: toId })
  });
  if (!res.ok) throw new Error('Failed to restore walkway');
  return res.json();
}

export async function restoreAllWalkways() {
  const res = await fetch(`${API_BASE}/walkway/restore-all`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' }
  });
  if (!res.ok) throw new Error('Failed to restore all walkways');
  return res.json();
}

export async function addLocation(locationData) {
  const res = await fetch(`${API_BASE}/locations`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(locationData)
  });
  if (!res.ok) throw new Error('Failed to add location');
  return res.json();
}

export async function addWalkway(walkwayData) {
  const res = await fetch(`${API_BASE}/connections`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(walkwayData)
  });
  if (!res.ok) throw new Error('Failed to add walkway');
  return res.json();
}

export async function fetchAlgorithmComparison(fromId, toId) {
  const res = await fetch(`${API_BASE}/compare?from=${fromId}&to=${toId}`);
  if (!res.ok) throw new Error('Failed to compare algorithms');
  return res.json();
}
